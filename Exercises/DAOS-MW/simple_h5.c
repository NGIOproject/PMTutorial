#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "hdf5.h"

int main(int argc, char **argv)
{
    hid_t fid, file_space, dset;
    herr_t ret;

    if (argc != 2) {
	    fprintf(stderr, "usage: ./prog filename\n");
	    exit (1);
    }

    fid = H5Fcreate(argv[1], H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    assert(fid >= 0);

    char    *wbuf = "hello world!";
    hsize_t dims[1] = {strlen(wbuf) + 1};
    file_space = H5Screate_simple(1, dims, NULL);
    assert(file_space >= 0);

    dset = H5Dcreate(fid, "DATASET1", H5T_NATIVE_CHAR, file_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT); 
    assert(dset  >= 0);

    ret = H5Dwrite(dset, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, wbuf);
    assert(ret == 0);

    char rbuf[1024];
    ret = H5Dread(dset, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuf);
    assert(ret == 0);
    printf("read back = %s\n", rbuf);

    H5Dclose(dset);
    H5Sclose(file_space);
    H5Fclose(fid);
    return (0);
}
