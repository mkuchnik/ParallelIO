/*
 * Tests the PIO library with multiple iosysids in use at the
 * same time.
 *
 * This is a simplified, C version of the fortran pio_iosystem_tests2.F90.
 *
 * Ed Hartnett
 */
#include <pio.h>
#include <pio_tests.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 2

/* The name of this test. */
#define TEST_NAME "test_iosystem2_simple"

/* Number of test files generated. */
#define NUM_FILES 2

/* Used to define netcdf test file. */
#define DIMLEN 1

/* Needed to init intracomm. */
#define STRIDE 1
#define BASE 0
#define REARRANGER 1

/* Run test. */
int main(int argc, char **argv)
{
    int my_rank; /* Zero-based rank of processor. */
    int ntasks; /* Number of processors involved in current execution. */
    int iosysid; /* The ID for the parallel I/O system. */
    int iosysid_world; /* The ID for the parallel I/O system. */
    int num_flavors; /* Number of PIO netCDF flavors in this build. */
    int flavor[NUM_FLAVORS]; /* iotypes for the supported netCDF IO flavors. */
    int ret; /* Return code. */
    MPI_Comm test_comm;

    /* Initialize test. */
    if ((ret = pio_test_init(argc, argv, &my_rank, &ntasks, TARGET_NTASKS,
			     &test_comm)))
        ERR(ERR_INIT);
    
    /* Test code runs on TARGET_NTASKS tasks. The left over tasks do
     * nothing. */
    if(my_rank < TARGET_NTASKS)
    {
        /* Figure out iotypes. */
        if ((ret = get_iotypes(&num_flavors, flavor)))
            ERR(ret);

        /* Split world into odd and even. */
        MPI_Comm newcomm;
        int even = my_rank % 2 ? 0 : 1;
        if ((ret = MPI_Comm_split(test_comm, even, 0, &newcomm)))
            MPIERR(ret);
        printf("%d newcomm = %d even = %d\n", my_rank, newcomm, even);

        /* Get size of new communicator. */
        int new_size;
        if ((ret = MPI_Comm_size(newcomm, &new_size)))
            MPIERR(ret);

        /* Initialize an intracomm for evens/odds. */
        if ((ret = PIOc_Init_Intracomm(newcomm, new_size, STRIDE, BASE, REARRANGER, &iosysid)))
            ERR(ret);

        /* Initialize an intracomm for all processes. */
        if ((ret = PIOc_Init_Intracomm(test_comm, TARGET_NTASKS, STRIDE, BASE, REARRANGER,
                                       &iosysid_world)))
            ERR(ret);

        int ncid;
        int ncid2;
        for (int i = 0; i < num_flavors; i++)
        {
            char fn[NUM_FILES][NC_MAX_NAME + 1];
            char dimname[NUM_FILES][NC_MAX_NAME + 1];

            /* Create the test files. */
            for (int f = 0; f < NUM_FILES; f++)
            {
                int lncid, dimid;

                /* Creat file and dimension name. */
                sprintf(fn[f], "pio_iosys_test_file%d.nc", f);
                sprintf(dimname[f], "dim_%d", f);

                if ((ret = PIOc_createfile(iosysid_world, &lncid, &flavor[i], fn[f], NC_CLOBBER)))
                    return ret;
                /* Define a dimension. */
                if ((ret = PIOc_def_dim(lncid, dimname[f], DIMLEN, &dimid)))
                    return ret;
                if ((ret = PIOc_enddef(lncid)))
                    return ret;
                if ((ret = PIOc_closefile(lncid)))
                    return ret;
            }

            /* Open the first file with world iosystem. */
            if ((ret = PIOc_openfile(iosysid_world, &ncid, &flavor[i], fn[0], PIO_WRITE)))
                return ret;

            /* Check the first file. */
            char dimname_in[NC_MAX_NAME + 1];
            if ((ret = PIOc_inq_dimname(ncid, 0, dimname_in)))
                return ret;
            printf("%d ncid dimname_in = %s should be %s\n", my_rank, dimname_in, dimname[0]);
            if (strcmp(dimname_in, dimname[0]))
                return ERR_WRONG;

            /* Check the other file with the other IO. */
            if (even)
            {
                if ((ret = PIOc_openfile(iosysid, &ncid2, &flavor[i], fn[1], PIO_WRITE)))
                    return ret;
                if ((ret = PIOc_inq_dimname(ncid2, 0, dimname_in)))
                    return ret;
                printf("%d ncid2 dimname_in = %s should be %s\n", my_rank, dimname_in, dimname[1]);
                if (strcmp(dimname_in, dimname[1]))
                    return ERR_WRONG;
            }

            /* Close the still-open files. */
            if ((ret = PIOc_closefile(ncid)))
                ERR(ret);
            if (even)
                if ((ret = PIOc_closefile(ncid2)))
                    ERR(ret);
        } /* next iotype */

        /* Finalize PIO odd/even intracomm. */
        if ((ret = PIOc_finalize(iosysid)))
            ERR(ret);


        /* Finalize PIO world intracomm. */
        if ((ret = PIOc_finalize(iosysid_world)))
            ERR(ret);
    }/* my_rank < TARGET_NTASKS */

    /* Finalize test. */
    printf("%d %s finalizing...\n", my_rank, TEST_NAME);
    if ((ret = pio_test_finalize(&test_comm)))
        return ERR_AWFUL;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);

    return 0;
}
