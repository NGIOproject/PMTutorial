/**
 * (C) Copyright 2020-2021 Intel Corporation.
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 */

#include <daos.h>

#define FAIL(fmt, ...)						\
do {								\
	fprintf(stderr, "" fmt " aborting\n", ## __VA_ARGS__);	\
	exit(1);						\
} while (0)

#define	ASSERT(cond, ...)					\
do {								\
	if (!(cond))						\
		FAIL(__VA_ARGS__);				\
} while (0)

static void
buf_render(char *buf, unsigned int buf_len)
{
	int	nr = 'z' - 'a' + 1;
	int	i;

	for (i = 0; i < buf_len - 1; i++) {
		int randv = rand() % (2 * nr);

		if (randv < nr)
			buf[i] = 'a' + randv;
		else
			buf[i] = 'A' + (randv - nr);
	}
	buf[i] = '\0';
}

#define KEYS 20
#define BUFLEN 1024
static daos_handle_t	poh;
static daos_handle_t	coh;

/** Buffer sizes for dkey enumeration */
#define ENUM_DESC_BUF	512
#define ENUM_DESC_NR	5

static void
list_keys(daos_handle_t oh, int *num_keys)
{
	char		buf[ENUM_DESC_BUF];
	daos_key_desc_t kds[ENUM_DESC_NR];
	daos_anchor_t	anchor = {0};
	int		key_nr = 0;
	d_sg_list_t	sgl;
	d_iov_t		sg_iov;

	d_iov_set(&sg_iov, buf, ENUM_DESC_BUF);
	sgl.sg_nr		= 1;
	sgl.sg_nr_out		= 0;
	sgl.sg_iovs		= &sg_iov;

	printf("Enumerate All keys, %d keys at a time\n", ENUM_DESC_NR);

	/** enumerate here */

	*num_keys = key_nr;
}

void
run_daos_kv()
{
	daos_handle_t	oh;
	char		buf[BUFLEN];
	daos_obj_id_t	oid;
	char		key[32] = {0};
	int		i, rc;

	oid.hi = 0;
	oid.lo = 4;
	/** the KV API requires the flat feature flag be set in the oid */
	daos_obj_generate_oid(coh, &oid, DAOS_OT_KV_HASHED, OC_SX, 0, 0);

	rc = daos_kv_open(coh, oid, DAOS_OO_RW, &oh, NULL);
	ASSERT(rc == 0, "KV open failed: %s (%d)", d_errstr(rc), rc);

	/** put keys */
	printf("Inserting %d Keys...\n", KEYS);
	for (i = 0; i < KEYS; i++) {
		buf_render(buf, BUFLEN);
		sprintf(key, "key_%d", i);
		rc = daos_kv_put(oh, DAOS_TX_NONE, 0, key, BUFLEN, buf, NULL);
		ASSERT(rc == 0, "KV put failed: %s (%d)", d_errstr(rc), rc);
	}

	int num_keys;

	/** Enumerate all Keys inserted */
	list_keys(oh, &num_keys);
	printf("%d keys enumerated\n", num_keys);

	rc = daos_kv_close(oh, NULL);
	ASSERT(rc == 0, "KV close failed");
}

#define CONT "cont3"
int
main(int argc, char **argv)
{
	int		rc;

	if (argc != 2) {
		fprintf(stderr, "args: pool\n");
		exit(1);
	}

	rc = daos_init();
	ASSERT(rc == 0, "daos_init failed: %s (%d)", d_errstr(rc), rc);

	rc = daos_pool_connect(argv[1], NULL, DAOS_PC_RW, &poh, NULL, NULL);
	ASSERT(rc == 0, "pool connect failed: %s (%d)", d_errstr(rc), rc);

	/** create the container */
	rc = daos_cont_create_with_label(poh, CONT, NULL, NULL, NULL);
	ASSERT(rc == 0, "container create failed: %s (%d)", d_errstr(rc), rc);

	/** open container */
	rc = daos_cont_open(poh, CONT, DAOS_COO_RW, &coh, NULL, NULL);
	ASSERT(rc == 0, "container open failed: %s (%d)", d_errstr(rc), rc);

	run_daos_kv();

	rc = daos_cont_close(coh, NULL);
	ASSERT(rc == 0, "cont close failed");

	rc = daos_cont_destroy(poh, CONT, 0, NULL);
	ASSERT(rc == 0, "cont destroy failed");

	rc = daos_pool_disconnect(poh, NULL);
	ASSERT(rc == 0, "disconnect failed");

	rc = daos_fini();
	ASSERT(rc == 0, "daos_fini failed: %s (%d)", d_errstr(rc), rc);

	return rc;
}
