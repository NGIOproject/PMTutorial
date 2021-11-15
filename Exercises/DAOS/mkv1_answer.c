/**
 * (C) Copyright 2020-2021 Intel Corporation.
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 */

#include <daos.h>

#define FAIL(fmt, ...)						\
do {								\
	fprintf(stderr, "" fmt " aborting\n", ## __VA_ARGS__);	\
	goto out;						\
} while (0)

#define	CHECK(cond, ...)					\
do {								\
	if (!(cond))						\
		FAIL(__VA_ARGS__);				\
} while (0)

#define BUFLEN 100
static daos_handle_t	poh;
static daos_handle_t	coh;

void
run_daos_mkv()
{
	daos_handle_t	oh;
	char		sv_buf[BUFLEN];
	char		sv_rbuf[BUFLEN];
	int		array_buf[BUFLEN];
	int		rbuf[BUFLEN];
	daos_obj_id_t	oid;
	int		rc, i;

	oid.hi = 0;
	oid.lo = 2;
	daos_obj_generate_oid(coh, &oid, 0, OC_SX, 0, 0);

	rc = daos_obj_open(coh, oid, DAOS_OO_RW, &oh, NULL);
	CHECK(rc == 0, "object open failed: %s (%d)", d_errstr(rc), rc);

	sprintf(sv_buf, "HELLO WORLD!");
	for (i = 0; i < BUFLEN; i++)
		array_buf[i] = i;

	d_sg_list_t	sgls[2];
	d_iov_t		sg_iovs[2];
	daos_iod_t	iods[2];
	daos_recx_t	recx;
	d_iov_t		dkey;
	char		dkey_str[] = "DKEY1";
	char		array_akey[] = "array_akey";
	char		sv_akey[] = "sv_akey";

	d_iov_set(&dkey, dkey_str, strlen(dkey_str));

	d_iov_set(&sg_iovs[0], array_buf, BUFLEN * sizeof(int));
	sgls[0].sg_nr		= 1;
	sgls[0].sg_nr_out	= 0;
	sgls[0].sg_iovs		= &sg_iovs[0];

	d_iov_set(&sg_iovs[1], sv_buf, strlen(sv_buf));
	sgls[1].sg_nr		= 1;
	sgls[1].sg_nr_out	= 0;
	sgls[1].sg_iovs		= &sg_iovs[1];

	/** Set IOD for each AKEY */

	/** ARRAY akey IOD */
	d_iov_set(&iods[0].iod_name, array_akey, strlen(array_akey));
	iods[0].iod_nr		= 1; /** 1 range */
	iods[0].iod_size	= sizeof(int); /** record size */
	recx.rx_nr		= BUFLEN; /** extent size */
	recx.rx_idx		= 0; /** extent offset */
	iods[0].iod_recxs	= &recx;
	iods[0].iod_type	= DAOS_IOD_ARRAY; /** value type of the akey */

	/** SV akey IOD */
	d_iov_set(&iods[1].iod_name, sv_akey, strlen(sv_akey));
	iods[1].iod_nr		= 1; /** has to be 1 for single value */
	iods[1].iod_size	= strlen(sv_buf); /** size of the single value */
	iods[1].iod_recxs	= NULL; /** recx is ignored for single value */
	iods[1].iod_type	= DAOS_IOD_SINGLE; /** value type of the akey */

	/** issue the update operation */
	rc = daos_obj_update(oh, DAOS_TX_NONE, 0, &dkey, 2, iods, sgls, NULL);
	CHECK(rc == 0, "object update failed: %s (%d)", d_errstr(rc), rc);


	/** read each akey individually */

	d_sg_list_t	sgl;
	d_iov_t		sg_iov;
	daos_iod_t	iod;

	/** init scatter/gather */
	d_iov_set(&sg_iov, rbuf, BUFLEN * sizeof(int));
	sgl.sg_nr		= 1;
	sgl.sg_nr_out		= 0;
	sgl.sg_iovs		= &sg_iov;

	/** init I/O descriptor */
	d_iov_set(&iod.iod_name, array_akey, strlen(array_akey));
	iod.iod_nr	= 1; /** number of extents in recx array */
	iod.iod_size	= sizeof(int); /** record size (1 byte array here) */
	recx.rx_nr	= BUFLEN; /** extent size */
	recx.rx_idx	= 0; /** extent offset */
	iod.iod_recxs	= &recx;
	iod.iod_type	= DAOS_IOD_ARRAY; /** value type of the akey */

	/** fetch a dkey */
	rc = daos_obj_fetch(oh, DAOS_TX_NONE, 0, &dkey, 1, &iod, &sgl, NULL, NULL);
	CHECK(rc == 0, "object fetch failed: %s (%d)", d_errstr(rc), rc);

	printf("Verifying Data from array akey ...\n");
	if (memcmp(array_buf, rbuf, BUFLEN * sizeof(int)) != 0)
		CHECK(0, "Data verification");

	/** init scatter/gather */
	d_iov_set(&sg_iov, sv_rbuf, BUFLEN);
	sgl.sg_nr		= 1;
	sgl.sg_nr_out		= 0;
	sgl.sg_iovs		= &sg_iov;

	/** init I/O descriptor */
	d_iov_set(&iod.iod_name, sv_akey, strlen(sv_akey));
	iod.iod_nr	= 1;
	iod.iod_size	= DAOS_REC_ANY;
	iod.iod_recxs	= NULL;
	iod.iod_type	= DAOS_IOD_SINGLE;

	/** fetch a dkey */
	rc = daos_obj_fetch(oh, DAOS_TX_NONE, 0, &dkey, 1, &iod, &sgl, NULL, NULL);
	CHECK(rc == 0, "object fetch failed: %s (%d)", d_errstr(rc), rc);

	printf("Verifying Data from SV akey ...\n");
	if (memcmp(sv_buf, sv_rbuf, strlen(sv_buf)) != 0)
		CHECK(0, "Data verification");
	sv_rbuf[iod.iod_size] = '\0';
	printf("Single Value size = %zu, buf = %s\n", iod.iod_size, sv_rbuf);
out:
	daos_obj_close(oh, NULL);
	return;
}

#define CONT "cont5"

int
main(int argc, char **argv)
{
	int		rc;

	if (argc != 2) {
		fprintf(stderr, "args: pool\n");
		exit(1);
	}

	rc = daos_init();
	CHECK(rc == 0, "daos_init failed: %s (%d)", d_errstr(rc), rc);

	rc = daos_pool_connect(argv[1], NULL, DAOS_PC_RW, &poh, NULL, NULL);
	CHECK(rc == 0, "pool connect failed: %s (%d)", d_errstr(rc), rc);

	/** create the container */
	rc = daos_cont_create_with_label(poh, CONT, NULL, NULL, NULL);
	CHECK(rc == 0, "container create failed: %s (%d)", d_errstr(rc), rc);

	/** open container */
	rc = daos_cont_open(poh, CONT, DAOS_COO_RW, &coh, NULL, NULL);
	CHECK(rc == 0, "container open failed: %s (%d)", d_errstr(rc), rc);

	run_daos_mkv();

out:
	daos_cont_close(coh, NULL);
	daos_cont_destroy(poh, CONT, 0, NULL);
	daos_pool_disconnect(poh, NULL);
	daos_fini();
	return rc;
}
