#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int ac, char **av)
{
        int  mpi_size, mpi_rank, mpi_err;
        MPI_File fh;
        char filename[24];
	char mpi_err_str[MPI_MAX_ERROR_STRING];
        int  mpi_err_strlen;
        MPI_Offset  mpi_off;
        MPI_Status  mpi_stat;

        MPI_Init(&ac, &av);
        MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
        MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

	sprintf(filename, "file_mpiio_%d", mpi_rank);
        if ((mpi_err = MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_RDWR | MPI_MODE_CREATE,
                                     MPI_INFO_NULL, &fh)) != MPI_SUCCESS) {
                MPI_Error_string(mpi_err, mpi_err_str, &mpi_err_strlen);
                printf("MPI_File_open failed (%s)\n", mpi_err_str);
                return 1;
        }

	char	*wbuf = "hello world!";

        mpi_off = 0;
        if ((mpi_err = MPI_File_write_at(fh, mpi_off, wbuf, strlen(wbuf) + 1, MPI_BYTE, &mpi_stat))
	    != MPI_SUCCESS){
                MPI_Error_string(mpi_err, mpi_err_str, &mpi_err_strlen);
                printf("MPI_File_write_at offset(%ld), failed (%s)\n", (long) mpi_off, mpi_err_str);
                return 1;
        }

	char	rbuf[1024];
	ssize_t	read_size;

	if ((mpi_err = MPI_File_read_at(fh, mpi_off, rbuf, sizeof(rbuf), MPI_INT, &mpi_stat))
	    != MPI_SUCCESS){
                MPI_Error_string(mpi_err, mpi_err_str, &mpi_err_strlen);
                printf("MPI_File_read_at offset(%ld), failed (%s)\n",
                       (long) mpi_off, mpi_err_str);
                return 1;
        }

	printf("Rank %d read back = %s\n", mpi_rank, rbuf);
        MPI_File_close(&fh);
        MPI_Finalize();
        return 0;
}
