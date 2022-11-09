#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

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
main(int ac, char **av)
{
        int	fd;
        char	*filename = "file_posix";
        int	rc;
	mode_t  create_mode = S_IWUSR | S_IRUSR;
	int     create_flags = O_RDWR | O_CREAT | O_EXCL;

        if((fd = open(filename, create_flags, create_mode)) < -1)
                return 1;

	char	*wbuf = "hello world!";

	pwrite(fd, wbuf, strlen(wbuf) + 1, 0);

	char	rbuf[1024];
	ssize_t	read_size;

	read_size = pread(fd, &rbuf, sizeof(rbuf), 0);
	ASSERT(read_size == strlen(wbuf) + 1, "not enough data read\n");
	printf("read back: %s\n", rbuf);
	close(fd);

        return 0;
}
