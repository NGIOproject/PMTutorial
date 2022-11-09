#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <daos.h>
#include <daos_fs.h>

#define FAIL(fmt, ...)						\
do {								\
	fprintf(stderr, fmt " aborting\n", ## __VA_ARGS__);	\
	exit(1);						\
} while (0)

#define	ASSERT(cond, ...)					\
do {								\
	if (!(cond))						\
		FAIL(__VA_ARGS__);				\
} while (0)


int
main(int argc, char **argv)
{
	dfs_t		*dfs;
	dfs_obj_t	*file;
	int		rc;

	if (argc != 3) {
		fprintf(stderr, "usage: ./exec pool cont\n");
		exit(1);
	}

	/** initialize the local DAOS stack */
	rc = dfs_init();
	ASSERT(rc == 0, "dfs_init failed with %d", rc);

	rc = dfs_connect(argv[1], NULL, argv[2], O_CREAT | O_RDWR, NULL, &dfs);
	ASSERT(rc == 0, "dfs_connect failed with %d", rc);

	mode_t	create_mode = S_IWUSR | S_IRUSR;
	int	create_flags = O_RDWR | O_CREAT | O_EXCL;

	/** create & open /file1 */
	rc = dfs_open(dfs, NULL, "file_dfs", create_mode | S_IFREG, create_flags, 0, 0, NULL, &file);
	ASSERT(rc == 0, "create /dir1/file failed\n");

	/** write a "hello world!" string to the file at offset 0 */

	char		*wbuf = "hello world!";
	d_sg_list_t     sgl;
	d_iov_t         iov;

	/** setup iovec (sgl in DAOS terms) for write buffer */
	d_iov_set(&iov, wbuf, strlen(wbuf) + 1);
	sgl.sg_nr = 1;
	sgl.sg_iovs = &iov;
	rc = dfs_write(dfs, file, &sgl, 0 /** offset */, NULL);
	ASSERT(rc == 0, "dfs_write() failed\n");

	char		rbuf[1024];
	daos_size_t	read_size;

	/** reset iovec for read buffer */
	d_iov_set(&iov, rbuf, sizeof(rbuf));
	rc = dfs_read(dfs, file, &sgl, 0 /** offset */, &read_size, NULL);
	ASSERT(rc == 0, "dfs_read() failed\n");
	ASSERT(read_size == strlen(wbuf) + 1, "not enough data read\n");
	printf("read back: %s\n", rbuf);

	/** close / finalize */
	dfs_release(file);

	rc = dfs_disconnect(dfs);
	ASSERT(rc == 0, "disconnect failed");
	rc = dfs_fini();
	ASSERT(rc == 0, "dfs_fini failed with %d", rc);

	return rc;
}
