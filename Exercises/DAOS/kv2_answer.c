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

#define KEYS 5
#define BUFLEN 1024
static daos_handle_t	poh;
static daos_handle_t	coh;

void
run_daos_kv()
{
	daos_handle_t	oh;
	char		buf[BUFLEN], rbuf[BUFLEN];
	daos_obj_id_t	oid;
	char		key[32] = {0};
	int		i, rc;

	oid.hi = 0;
	oid.lo = 4;
	/** the KV API requires the flat feature flag be set in the oid */
	daos_obj_generate_oid(coh, &oid, DAOS_OT_KV_HASHED, OC_SX, 0, 0);

	rc = daos_kv_open(coh, oid, DAOS_OO_RW, &oh, NULL);
	ASSERT(rc == 0, "KV open failed: %s (%d)", d_errstr(rc), rc);

	/** put keys with random size */
	for (i = 0; i < KEYS; i++) {
		daos_size_t size;

		buf_render(buf, BUFLEN);
		sprintf(key, "key_%d", i);

		size = BUFLEN / (i + 1);
		printf("Insert Key %d: %s, size = %zu\n", i, key, size);

		rc = daos_kv_put(oh, DAOS_TX_NONE, 0, key, size, buf, NULL);
		ASSERT(rc == 0, "KV put failed: %s (%d)", d_errstr(rc), rc);
	}

	/** read keys */
	for (i = 0; i < KEYS; i++) {
		daos_size_t size;

		sprintf(key, "key_%d", i);
		printf("Get Size and Value of Key %d: %s\n", i, key);

		/** first query the size */
		rc = daos_kv_get(oh, DAOS_TX_NONE, 0, key, &size, NULL, NULL);
		ASSERT(rc == 0, "KV get failed: %s (%d)", d_errstr(rc), rc);
		printf("%s: value size = %zu\n", key, size);

		/** get the data */
		rc = daos_kv_get(oh, DAOS_TX_NONE, 0, key, &size, rbuf, NULL);
		ASSERT(rc == 0, "KV get failed: %s (%d)", d_errstr(rc), rc);
	}

	rc = daos_kv_close(oh, NULL);
	ASSERT(rc == 0, "KV close failed");
	printf("SUCCESS\n");
}

#define CONT "cont2"

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
