/*
 * Tests for NetCDF-4 Functions.
 *
 * There are some functions that apply only to netCDF-4 files. This
 * test checks those functions. PIO will return an error if these
 * functions are called on non-netCDF-4 files, and that is tested in
 * this code as well.
 *
 * Ed Hartnett
 */
#include <pio.h>
#include <pio_tests.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 4

/* The minimum number of tasks this test should run on. */
#define MIN_NTASKS 4

/* The name of this test. */
#define TEST_NAME "test_nc4"

/* Number of processors that will do IO. */
#define NUM_IO_PROCS 1

/* Number of computational components to create. */
#define COMPONENT_COUNT 1

/* The number of dimensions in the example data. In this test, we
 * are using three-dimensional data. */
#define NDIM 3

/* The length of our sample data along each dimension. */
#define X_DIM_LEN 400
#define Y_DIM_LEN 400

/* The number of timesteps of data to write. */
#define NUM_TIMESTEPS 6

/* The name of the variable in the netCDF output file. */
#define VAR_NAME "foo"

/* The meaning of life, the universe, and everything. */
#define START_DATA_VAL 42

/* Values for some netcdf-4 settings. */
#define VAR_CACHE_SIZE (1024 * 1024)
#define VAR_CACHE_NELEMS 10
#define VAR_CACHE_PREEMPTION 0.5

/* The dimension names. */
char dim_name[NDIM][NC_MAX_NAME + 1] = {"timestep", "x", "y"};

/* Length of the dimensions in the sample data. */
int dim_len[NDIM] = {NC_UNLIMITED, X_DIM_LEN, Y_DIM_LEN};

/* Length of chunksizes to use in netCDF-4 files. */
PIO_Offset chunksize[NDIM] = {2, X_DIM_LEN/2, Y_DIM_LEN/2};

int test_deletefile(int iosysid, int num_flavors, int *flavor, int my_rank)
{
    int ncid;
    int ret;    /* Return code. */
    
    /* Use PIO to create the example file in each of the four
     * available ways. */
    for (int fmt = 0; fmt < num_flavors; fmt++)
    {
        char filename[NC_MAX_NAME + 1]; /* Test filename. */
        char iotype_name[NC_MAX_NAME + 1];

        /* Set error handling. */
        PIOc_Set_IOSystem_Error_Handling(iosysid, PIO_RETURN_ERROR);

        /* Create a filename. */
        if ((ret = get_iotype_name(flavor[fmt], iotype_name)))
            return ret;
        sprintf(filename, "delete_me_%s_%s.nc", TEST_NAME, iotype_name);

        printf("%d testing delete for file %s with format %d...\n",
               my_rank, filename, flavor[fmt]);
        if ((ret = PIOc_createfile(iosysid, &ncid, &(flavor[fmt]), filename, PIO_CLOBBER)))
            ERR(ret);

        /* End define mode. */
        if ((ret = PIOc_enddef(ncid)))
            ERR(ret);

        /* Close the netCDF file. */
        printf("%d Closing the sample data file...\n", my_rank);
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);

        /* Now delete the file. */
        printf("%d Deleting %s...\n", my_rank, filename);
        if ((ret = PIOc_deletefile(iosysid, filename)))
            ERR(ret);

        /* Make sure it is gone. */
        /* if ((ret = PIOc_openfile(iosysid, &ncid, &(flavor[fmt]), filename, */
        /*                          PIO_NOWRITE)) != PIO_ENFILE) */
        /*     ERR(ret); */
        
    }

    return PIO_NOERR;
}

/* Test some put and get functions. */
int test_put_get(int iosysid, int num_flavors, int *flavor, int my_rank)
{
    int ncid;
    int varid;
    int dimid[NDIM];
    int ret;    /* Return code. */
    
    /* Use PIO to create the example file in each of the four
     * available ways. */
    for (int fmt = 0; fmt < num_flavors; fmt++)
    {
        char filename[NC_MAX_NAME + 1]; /* Test filename. */
        char iotype_name[NC_MAX_NAME + 1];

        /* Set error handling. */
        PIOc_Set_IOSystem_Error_Handling(iosysid, PIO_RETURN_ERROR);

        /* Create a filename. */
        if ((ret = get_iotype_name(flavor[fmt], iotype_name)))
            return ret;
        sprintf(filename, "put_get_%s_%s.nc", TEST_NAME, iotype_name);

        printf("%d testing delete for file %s with format %d...\n",
               my_rank, filename, flavor[fmt]);
        if ((ret = PIOc_createfile(iosysid, &ncid, &(flavor[fmt]), filename, PIO_CLOBBER)))
            ERR(ret);

        /* Create dimensions. */
        for (int d = 0; d < NDIM; d++)
            if ((ret = PIOc_def_dim(ncid, dim_name[d], dim_len[d], &dimid[d])))
                ERR(ret);

        /* Create variable. */
        printf("%d Defining netCDF variable %s, ndims %d\n", my_rank, VAR_NAME, NDIM);
        if ((ret = PIOc_def_var(ncid, VAR_NAME, PIO_FLOAT, NDIM, dimid, &varid)))
            ERR(ret);

        /* End define mode. */
        if ((ret = PIOc_enddef(ncid)))
            ERR(ret);

        /* Write some data. */
        {
            PIO_Offset start[NDIM] = {0, 0, 0};
            PIO_Offset count[NDIM] = {1, 1, 1};
            float data = 42.42;
            
            if ((ret = PIOc_put_vara_float(ncid, varid, start, count, &data)))
                ERR(ret);
        }

        /* Close the netCDF file. */
        printf("%d Closing the sample data file...\n", my_rank);
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);

        /* Reopen the file. */
        if ((ret = PIOc_openfile(iosysid, &ncid, &(flavor[fmt]), filename, PIO_NOWRITE)))
            ERR(ret);

        /* Check the dimensions. */
        for (int d = 0; d < NDIM; d++)
        {
            char dim_name_in[PIO_MAX_NAME + 1];
            PIO_Offset dim_len_in;
            if ((ret = PIOc_inq_dim(ncid, d, dim_name_in, &dim_len_in)))
                ERR(ret);
            if (strncmp(dim_name_in, dim_name[d], PIO_MAX_NAME) || dim_len_in != dim_len[d])
                ERR(ERR_WRONG);
        }

        /* Check the variable. */
        {
            char var_name_in[PIO_MAX_NAME + 1];
            nc_type xtype_in;
            int ndims_in, dimids_in[NDIM], natts_in;
            
            if ((ret = PIOc_inq_var(ncid, varid, var_name_in, &xtype_in, &ndims_in, dimids_in, &natts_in)))
                ERR(ret);
            if (strncmp(var_name_in, VAR_NAME, PIO_MAX_NAME) || xtype_in != PIO_FLOAT || ndims_in != NDIM ||
                natts_in != 0)
                ERR(ERR_WRONG);
            for (int d = 0; d < NDIM; d++)
                if (dimids_in[d] != dimid[d])
                    ERR(ERR_WRONG);
        }

        /* Close the file. */
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);
    }

    return PIO_NOERR;
}

/* Test the netCDF-4 optimization functions. */
int test_nc4(int iosysid, int num_flavors, int *flavor, int my_rank)
{
    int ncid;    /* The ncid of the netCDF file. */
    int dimids[NDIM];    /* The dimension IDs. */
    int varid;    /* The ID of the netCDF varable. */

    /* For setting the chunk cache. */
    PIO_Offset chunk_cache_size = 1024*1024;
    PIO_Offset chunk_cache_nelems = 1024;
    float chunk_cache_preemption = 0.5;

    /* For reading the chunk cache. */
    PIO_Offset chunk_cache_size_in;
    PIO_Offset chunk_cache_nelems_in;
    float chunk_cache_preemption_in;

    int storage = NC_CHUNKED; /* Storage of netCDF-4 files (contiguous vs. chunked). */
    PIO_Offset my_chunksize[NDIM]; /* Chunksizes we get from file. */
    int shuffle;    /* The shuffle filter setting in the netCDF-4 test file. */
    int deflate;    /* Non-zero if deflate set for the variable in the netCDF-4 test file. */
    int deflate_level;    /* The deflate level set for the variable in the netCDF-4 test file. */
    int endianness;    /* Endianness of variable. */
    PIO_Offset var_cache_size;    /* Size of the var chunk cache. */
    PIO_Offset var_cache_nelems; /* Number of elements in var cache. */
    float var_cache_preemption;     /* Var cache preemption. */
    char varname_in[NC_MAX_NAME];
    int expected_ret; /* The return code we expect to get. */
    int ret;    /* Return code. */

    /* Use PIO to create the example file in each of the four
     * available ways. */
    for (int fmt = 0; fmt < num_flavors; fmt++)
    {
        char filename[NC_MAX_NAME + 1]; /* Test filename. */
        char iotype_name[NC_MAX_NAME + 1];

        /* Create a filename. */
        if ((ret = get_iotype_name(flavor[fmt], iotype_name)))
            return ret;
        sprintf(filename, "%s_%s.nc", TEST_NAME, iotype_name);

        printf("%d Setting chunk cache for file %s with format %d...\n",
               my_rank, filename, flavor[fmt]);

        /* Try to set the chunk cache with invalid preemption to check
         * error handling. Can't do this because correct bahavior is
         * to MPI_Abort, and code now does that. But how to test? */
        /* chunk_cache_preemption = 50.0; */
        /* ret = PIOc_set_chunk_cache(iosysid, flavor[fmt], chunk_cache_size, */
        /*                            chunk_cache_nelems, chunk_cache_preemption); */
        
        /* printf("%d Set chunk cache ret = %d.\n", my_rank, ret); */

        /* /\* What result did we expect to get? *\/ */
        /* expected_ret = flavor[fmt] == PIO_IOTYPE_NETCDF4C || flavor[fmt] == PIO_IOTYPE_NETCDF4P ? */
        /*     NC_EINVAL : NC_ENOTNC4; */
        
        /* /\* Check the result. *\/ */
        /* if (ret != expected_ret) */
        /*     ERR(ERR_AWFUL); */

        /* Try to set the chunk cache for netCDF-4 iotypes. */
        chunk_cache_preemption = 0.5;
        if (flavor[fmt] == PIO_IOTYPE_NETCDF4C || flavor[fmt] == PIO_IOTYPE_NETCDF4P)
            if ((ret = PIOc_set_chunk_cache(iosysid, flavor[fmt], chunk_cache_size,
                                            chunk_cache_nelems, chunk_cache_preemption)))
                ERR(ERR_AWFUL);

        /* Now check the chunk cache. */
        if (flavor[fmt] == PIO_IOTYPE_NETCDF4C || flavor[fmt] == PIO_IOTYPE_NETCDF4P)
        {
            if ((ret = PIOc_get_chunk_cache(iosysid, flavor[fmt], &chunk_cache_size_in,
                                            &chunk_cache_nelems_in, &chunk_cache_preemption_in)))
                ERR(ERR_AWFUL);

            /* Check that we got the correct values. */
            if (chunk_cache_size_in != chunk_cache_size || chunk_cache_nelems_in != chunk_cache_nelems ||
                chunk_cache_preemption_in != chunk_cache_preemption)
                ERR(ERR_AWFUL);
        }

        /* Create the netCDF output file. */
        printf("%d Creating sample file %s with format %d...\n",
               my_rank, filename, flavor[fmt]);
        if ((ret = PIOc_createfile(iosysid, &ncid, &(flavor[fmt]), filename, PIO_CLOBBER)))
            ERR(ret);

        /* Set error handling. */
        /* PIOc_Set_File_Error_Handling(ncid, PIO_BCAST_ERROR); */

        /* Define netCDF dimensions and variable. */
        printf("%d Defining netCDF metadata...\n", my_rank);
        for (int d = 0; d < NDIM; d++)
        {
            printf("%d Defining netCDF dimension %s, length %d\n", my_rank,
                   dim_name[d], dim_len[d]);
            if ((ret = PIOc_def_dim(ncid, dim_name[d], (PIO_Offset)dim_len[d], &dimids[d])))
                ERR(ret);
        }
        printf("%d Defining netCDF variable %s, ndims %d\n", my_rank, VAR_NAME, NDIM);
        if ((ret = PIOc_def_var(ncid, VAR_NAME, PIO_FLOAT, NDIM, dimids, &varid)))
            ERR(ret);

        /* For netCDF-4 files, set the chunksize to improve performance. */
        if (flavor[fmt] == PIO_IOTYPE_NETCDF4C || flavor[fmt] == PIO_IOTYPE_NETCDF4P)
        {
            printf("%d Defining chunksizes\n", my_rank);
            if ((ret = PIOc_def_var_chunking(ncid, 0, NC_CHUNKED, chunksize)))
                ERR(ret);

            /* Check that the inq_varname function works. */
            printf("%d Checking varname\n", my_rank);
            ret = PIOc_inq_varname(ncid, 0, varname_in);
            printf("%d ret: %d varname_in: %s\n", my_rank, ret, varname_in);

            /* Check that the inq_var_chunking function works. */
            printf("%d Checking chunksizes\n", my_rank);
            if ((ret = PIOc_inq_var_chunking(ncid, 0, &storage, my_chunksize)))
                ERR(ret);
            printf("%d ret: %d storage: %d\n", my_rank, ret, storage);
            for (int d1 = 0; d1 < NDIM; d1++)
                printf("chunksize[%d] = %d\n", d1, my_chunksize[d1]);

            /* Check the answers. */
            if (storage != NC_CHUNKED)
                ERR(ERR_AWFUL);
            for (int d1 = 0; d1 < NDIM; d1++)
                if (my_chunksize[d1] != chunksize[d1])
                    ERR(ERR_AWFUL);

            /* Check that the inq_var_deflate functions works. */
            if ((ret = PIOc_inq_var_deflate(ncid, 0, &shuffle, &deflate, &deflate_level)))
                ERR(ret);

            /* For serial netCDF-4 deflate is turned on by default */
            if (flavor[fmt] == PIO_IOTYPE_NETCDF4C)
                if (shuffle || !deflate || deflate_level != 1)
                    ERR(ERR_AWFUL);

            /* For parallel netCDF-4, no compression available. :-( */
            if (flavor[fmt] == PIO_IOTYPE_NETCDF4P)
                if (shuffle || deflate)
                    ERR(ERR_AWFUL);

            /* Check setting the chunk cache for the variable. */
            printf("%d PIOc_set_var_chunk_cache...\n", my_rank);
            if ((ret = PIOc_set_var_chunk_cache(ncid, 0, VAR_CACHE_SIZE, VAR_CACHE_NELEMS,
                                                VAR_CACHE_PREEMPTION)))
                ERR(ret);

            /* Check getting the chunk cache values for the variable. */
            printf("%d PIOc_get_var_chunk_cache...\n", my_rank);
            if ((ret = PIOc_get_var_chunk_cache(ncid, 0, &var_cache_size, &var_cache_nelems,
                                                &var_cache_preemption)))
                ERR(ret);

            /* Check that we got expected values. */
            printf("%d var_cache_size = %d\n", my_rank, var_cache_size);
            if (var_cache_size != VAR_CACHE_SIZE)
                ERR(ERR_AWFUL);
            if (var_cache_nelems != VAR_CACHE_NELEMS)
                ERR(ERR_AWFUL);
            if (var_cache_preemption != VAR_CACHE_PREEMPTION)
                ERR(ERR_AWFUL);

            if ((ret = PIOc_def_var_endian(ncid, 0, 1)))
                ERR(ERR_AWFUL);
            if ((ret = PIOc_inq_var_endian(ncid, 0, &endianness)))
                ERR(ERR_AWFUL);
            if (endianness != 1)
                ERR(ERR_WRONG);
            
    /*     } */
    /*     else */
    /*     { */
    /*         /\* Trying to set or inq netCDF-4 settings for non-netCDF-4 */
    /*          * files results in the PIO_ENOTNC4 error. *\/ */
    /*         if ((ret = PIOc_def_var_chunking(ncid, 0, NC_CHUNKED, chunksize)) != PIO_ENOTNC4) */
    /*             ERR(ERR_AWFUL); */
    /*         /\* if ((ret = PIOc_inq_var_chunking(ncid, 0, &storage, my_chunksize)) != PIO_ENOTNC4) *\/ */
    /*         /\*     ERR(ERR_AWFUL); *\/ */
    /*         if ((ret = PIOc_inq_var_deflate(ncid, 0, &shuffle, &deflate, &deflate_level)) */
    /*             != PIO_ENOTNC4) */
    /*             ERR(ret); */
    /*         if ((ret = PIOc_def_var_endian(ncid, 0, 1)) != PIO_ENOTNC4) */
    /*             ERR(ERR_AWFUL); */
    /*         if ((ret = PIOc_inq_var_endian(ncid, 0, &endianness)) != PIO_ENOTNC4) */
    /*             ERR(ERR_AWFUL); */
    /*         if ((ret = PIOc_set_var_chunk_cache(ncid, 0, VAR_CACHE_SIZE, VAR_CACHE_NELEMS, */
    /*                                             VAR_CACHE_PREEMPTION)) != PIO_ENOTNC4) */
    /*             ERR(ret); */
    /*         if ((ret = PIOc_get_var_chunk_cache(ncid, 0, &var_cache_size, &var_cache_nelems, */
    /*                                             &var_cache_preemption)) != PIO_ENOTNC4) */
    /*             ERR(ret); */
    /*         if ((ret = PIOc_set_chunk_cache(iosysid, flavor[fmt], chunk_cache_size, chunk_cache_nelems, */
    /*                                         chunk_cache_preemption)) != PIO_ENOTNC4) */
    /*             ERR(ret); */
    /*         if ((ret = PIOc_get_chunk_cache(iosysid, flavor[fmt], &chunk_cache_size, */
    /*                                         &chunk_cache_nelems, &chunk_cache_preemption)) != PIO_ENOTNC4) */
    /*             ERR(ret); */
        }

        /* End define mode. */
        if ((ret = PIOc_enddef(ncid)))
            ERR(ret);

        /* Close the netCDF file. */
        printf("%d Closing the sample data file...\n", my_rank);
        if ((ret = PIOc_closefile(ncid)))
            ERR(ret);
    }
    return PIO_NOERR;
}

int test_all(int iosysid, int num_flavors, int *flavor, int my_rank)
{
    int ret; /* Return code. */
    
    /* Test file deletes. */
    printf("%d testing deletefile\n", my_rank);
    if ((ret = test_deletefile(iosysid, num_flavors, flavor, my_rank)))
        return ret;

    /* Test some reads and writes. */
    printf("%d testing put and get\n", my_rank);
    if ((ret = test_put_get(iosysid, num_flavors, flavor, my_rank)))
        return ret;
    
    /* Test netCDF-4 functions. */
    printf("%d testing netcdf-4 functions\n", my_rank);
    if ((ret = test_nc4(iosysid, num_flavors, flavor, my_rank)))
        return ret;

    return PIO_NOERR;
}

/* Test without async.
 *
 * @param my_rank rank of the task.
 * @param num_flavors the number of PIO IO types that will be tested.
 * @param flavors array of the PIO IO types that will be tested.
 * @param test_comm communicator with all test tasks.
 * @returns 0 for success error code otherwise.
 */
int test_no_async(int my_rank, int num_flavors, int *flavor, MPI_Comm test_comm)
{
    int niotasks;    /* Number of processors that will do IO. */
    int ioproc_stride = 1;    /* Stride in the mpi rank between io tasks. */
    int numAggregator = 0;    /* Number of the aggregator? Always 0 in this test. */
    int ioproc_start = 0;     /* Zero based rank of first processor to be used for I/O. */
    PIO_Offset elements_per_pe; /* Array index per processing unit. */
    int iosysid;  /* The ID for the parallel I/O system. */
    int ioid;     /* The I/O description ID. */
    PIO_Offset *compdof; /* The decomposition mapping. */
    int ret;      /* Return code. */

    /* keep things simple - 1 iotask per MPI process */
    niotasks = TARGET_NTASKS;

    /* Initialize the PIO IO system. This specifies how
     * many and which processors are involved in I/O. */
    if ((ret = PIOc_Init_Intracomm(test_comm, niotasks, ioproc_stride,
                                   ioproc_start, PIO_REARR_SUBSET, &iosysid)))
        ERR(ret);

    /* Describe the decomposition. This is a 1-based array, so add 1! */
    elements_per_pe = X_DIM_LEN * Y_DIM_LEN / TARGET_NTASKS;
    if (!(compdof = malloc(elements_per_pe * sizeof(PIO_Offset))))
        return PIO_ENOMEM;
    for (int i = 0; i < elements_per_pe; i++)
        compdof[i] = my_rank * elements_per_pe + i + 1;

    /* Create the PIO decomposition for this test. */
    printf("%d Creating decomposition...\n", my_rank);
    if ((ret = PIOc_InitDecomp(iosysid, PIO_FLOAT, 2, &dim_len[1], (PIO_Offset)elements_per_pe,
                               compdof, &ioid, NULL, NULL, NULL)))
        ERR(ret);
    free(compdof);

    /* Run tests. */
    if ((ret = test_all(iosysid, num_flavors, flavor, my_rank)))
        return ret;
        
    /* Free the PIO decomposition. */
    printf("%d Freeing PIO decomposition...\n", my_rank);
    if ((ret = PIOc_freedecomp(iosysid, ioid)))
        ERR(ret);
    return PIO_NOERR;
}

/* Test with async.
 *
 * @param my_rank rank of the task.
 * @param nprocs the size of the communicator.
 * @param num_flavors the number of PIO IO types that will be tested.
 * @param flavors array of the PIO IO types that will be tested.
 * @param test_comm communicator with all test tasks.
 * @returns 0 for success error code otherwise.
 */
int test_async(int my_rank, int num_flavors, int *flavor, MPI_Comm test_comm)
{
    int niotasks;            /* Number of processors that will do IO. */
    int ioproc_stride = 1;   /* Stride in the mpi rank between io tasks. */
    int ioproc_start = 0;    /* 0 based rank of first task to be used for I/O. */
    PIO_Offset elements_per_pe;    /* Array index per processing unit. */
    int iosysid[COMPONENT_COUNT];  /* The ID for the parallel I/O system. */
    int ioid;                      /* The I/O description ID. */
    PIO_Offset *compdof;           /* The decomposition mapping. */
    int num_procs[COMPONENT_COUNT + 1] = {1, TARGET_NTASKS - 1}; /* Num procs in each component. */
    int mpierr;  /* Return code from MPI functions. */
    int ret;     /* Return code. */

    /* Is the current process a computation task? */
    int comp_task = my_rank < NUM_IO_PROCS ? 0 : 1;
    printf("%d comp_task = %d\n", my_rank, comp_task);

    /* Initialize the IO system. */
    if ((ret = PIOc_Init_Async(test_comm, NUM_IO_PROCS, NULL, COMPONENT_COUNT,
                               num_procs, NULL, iosysid)))
        ERR(ERR_INIT);
    for (int c = 0; c < COMPONENT_COUNT; c++)
        printf("%d iosysid[%d] = %d\n", my_rank, c, iosysid[c]);

    /* All the netCDF calls are only executed on the computation
     * tasks. The IO tasks have not returned from PIOc_Init_Intercomm,
     * and when the do, they should go straight to finalize. */
    if (comp_task)
    {
        /* Test the netCDF-4 functions. */
        if ((ret = test_nc4(iosysid[0], num_flavors, flavor, my_rank)))
            return ret;

        /* Finalize the IO system. Only call this from the computation tasks. */
        printf("%d %s Freeing PIO resources\n", my_rank, TEST_NAME);
        for (int c = 0; c < COMPONENT_COUNT; c++)
        {
            if ((ret = PIOc_finalize(iosysid[c])))
                ERR(ret);
            printf("%d %s PIOc_finalize completed for iosysid = %d\n", my_rank, TEST_NAME,
                   iosysid[c]);
        }
    } /* endif comp_task */

    return PIO_NOERR;
}

/* Run Tests for NetCDF-4 Functions. */
int main(int argc, char **argv)
{
    int my_rank; /* Zero-based rank of processor. */
    int ntasks; /* Number of processors involved in current execution. */
    int num_flavors; /* Number of PIO netCDF flavors in this build. */
    int flavor[NUM_FLAVORS]; /* iotypes for the supported netCDF IO flavors. */
    int ret;    /* Return code. */

    MPI_Comm test_comm; /* A communicator for this test. */

    /* Initialize test. */
    if ((ret = pio_test_init2(argc, argv, &my_rank, &ntasks, MIN_NTASKS,
                              TARGET_NTASKS, &test_comm)))
        ERR(ERR_INIT);

    /* Only do something on TARGET_NTASKS tasks. */
    if (my_rank < TARGET_NTASKS)
    {
        /* Figure out iotypes. */
        if ((ret = get_iotypes(&num_flavors, flavor)))
            ERR(ret);

        /* Run tests without async feature. */
        if ((ret = test_no_async(my_rank, num_flavors, flavor, test_comm)))
            return ret;

        /* Run tests with async. */
        /* if ((ret = test_async(my_rank, num_flavors, flavor, test_comm))) */
        /*     return ret; */

    } /* endif my_rank < TARGET_NTASKS */

    /* Finalize the MPI library. */
    printf("%d %s Finalizing...\n", my_rank, TEST_NAME);
    if ((ret = pio_test_finalize(&test_comm)))
        return ret;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);

    return 0;
}
