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
run_daos_array()
{
	daos_handle_t	oh;
	int		buf[BUFLEN], rbuf[BUFLEN];
	daos_obj_id_t	oid;
	int		rc;

	oid.hi = 0;
	oid.lo = 3;

	daos_array_generate_oid(coh, &oid, true, 0, 0, 0);

	/** Create the array object with cell size 4 (integer array) and 16 chunk size. */
	rc = daos_array_create(coh, oid, DAOS_TX_NONE, sizeof(int), 16, &oh, NULL);
	CHECK(rc == 0, "array create failed: %s (%d)", d_errstr(rc), rc);


	/** write 100 integers to the array */
	daos_array_iod_t iod;
	d_sg_list_t	sgl;
	daos_range_t	rg;
	d_iov_t		iov;
	daos_size_t	array_size;
	int		i;

	for (i = 0; i < BUFLEN; i++)
		buf[i] = i;

	/** set array iod */
	iod.arr_nr = 1; /** number of ranges / array iovec */
	rg.rg_len = BUFLEN; /** length (in records) */
	rg.rg_idx = 0; /** offset */
	iod.arr_rgs = &rg;

	/** set memory location */
	sgl.sg_nr = 1;
	d_iov_set(&iov, buf, BUFLEN * sizeof(int));
	sgl.sg_iovs = &iov;

	printf("Writing %d Integers to Array\n", BUFLEN);
	/** Write */
	rc = daos_array_write(oh, DAOS_TX_NONE, &iod, &sgl, NULL);
	CHECK(rc == 0, "array write failed: %s (%d)", d_errstr(rc), rc);

	/** check size */
	printf("Checking Array size:\n");
	rc = daos_array_get_size(oh, DAOS_TX_NONE, &array_size, NULL);
	CHECK(rc == 0, "array get_size failed: %s (%d)", d_errstr(rc), rc);
	CHECK(array_size == BUFLEN, "size check failed");
	printf("Array Size = %zu\n", array_size);

	d_iov_set(&iov, rbuf, BUFLEN * sizeof(int));
	sgl.sg_iovs = &iov;

	printf("Reading %d Integers from Array\n", BUFLEN);
	/** read & verify */
	rc = daos_array_read(oh, DAOS_TX_NONE, &iod, &sgl, NULL);
	CHECK(rc == 0, "array read failed: %s (%d)", d_errstr(rc), rc);

	printf("Verifying Data read ...\n");
	if (memcmp(buf, rbuf, BUFLEN * sizeof(int)) != 0)
		CHECK(0, "Data verification");

	daos_array_close(oh, NULL);
	printf("SUCCESS\n");
out:
	return;
}

#define CONT "cont4"

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

	run_daos_array();

out:
	daos_cont_close(coh, NULL);
	daos_cont_destroy(poh, CONT, 0, NULL);
	daos_pool_disconnect(poh, NULL);
	daos_fini();
	return rc;
}
