/**
 * @file 
 * Compute start and count arrays for the box rearranger
 *
 * @author Jim Edwards
 * @date  2014
 */
#include <config.h>

#ifdef TESTCALCDECOMP
enum PIO_DATATYPE{
    PIO_REAL,
    PIO_INT,
    PIO_DOUBLE
};
#define PIO_NOERR 0
typedef long long PIO_Offset;
#define max(a,b)                                \
    ({ __typeof__ (a) _a = (a);                 \
        __typeof__ (b) _b = (b);                \
        _a > _b ? _a : _b; })

#define min(a,b)                                \
    ({ __typeof__ (a) _a = (a);                 \
        __typeof__ (b) _b = (b);                \
        _a < _b ? _a : _b; })
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#else
#include <pio.h>
#include <pio_internal.h>
#endif

/** The default target blocksize for each io task when the box
 * rearranger is used. */
#define default_blocksize 1024

/** The target blocksize for each io task when the box rearranger is
 * used. */
int blocksize = default_blocksize;

/**
 * Set the target blocksize for the box rearranger.
 *
 * @param newblocksize the new blocksize.
 * @returns 0 for success.
 * @ingroup PIO_set_blocksize
 */
int PIOc_set_blocksize(int newblocksize)
{
    if (newblocksize > 0)
        blocksize = newblocksize;
    return PIO_NOERR;
}

/* Recursive Standard C Function: Greatest Common Divisor.
 *
 * @param a
 * @param b
 * @returns greates common divisor. */
int gcd(int a, int b )
{
    if (a == 0)
        return b;
    return gcd(b % a, a);
}

/* Recursive Standard C Function: Greatest Common Divisor for 64 bit
 * ints.
 *
 * @param a
 * @param b
 * @returns greates common divisor. */
long long lgcd(long long a, long long b)
{
    if (a == 0)
        return b;
    return lgcd(b % a, a);
}

/**
 * Return the gcd of nain and any value in ain.
 *
 * @param main
 * @param ain
 * @returns
 */
int gcd_array(int nain, int *ain)
{
    int i;
    int bsize = 1;

    for (i = 0; i < nain; i++)
        if (ain[i] <= 1)
            return bsize;

    bsize = ain[0];
    i = 1;
    while(i < nain && bsize > 1)
    {
        bsize = gcd(bsize,ain[i]);
        i++;
    }

    return bsize;
}

/**
 * Return the gcd of nain and any value in ain as int_64.
 *
 * @param main
 * @param ain
 * @returns
 */
long long lgcd_array(int nain, long long *ain)
{
    int i;
    long long bsize = 1;

    for (i = 0; i < nain; i++)
        if(ain[i] <= 1)
            return bsize;

    bsize = ain[0];
    i = 1;
    while (i < nain && bsize > 1)
    {
        bsize = gcd(bsize, ain[i]);
        i++;
    }

    return bsize;
}

/**
 * ???
 *
 * @param gdim
 * @param ioprocs
 * @param rank
 * @param start
 * @param kount
 */
void computestartandcount(int gdim, int ioprocs, int rank, PIO_Offset *start,
                          PIO_Offset *kount)
{
    int irank;
    int remainder;
    int adds;
    PIO_Offset lstart, lkount;

    irank = rank % ioprocs;
    lkount = (long int)(gdim / ioprocs);
    lstart = (long int)(lkount * irank);
    remainder = gdim - lkount * ioprocs;

    if (remainder >= ioprocs-irank)
    {
        lkount++;
        if ((adds = irank + remainder - ioprocs) > 0)
            lstart += adds;
    }
    *start = lstart;
    *kount = lkount;
}

/**
 * Look for the largest block of data for io which can be expressed in
 * terms of start and count.
 *
 * @param arrlen
 * @param arr_in
 * @returns
 */
PIO_Offset GCDblocksize(int arrlen, const PIO_Offset *arr_in)
{
    PIO_Offset *gaps = NULL;
    PIO_Offset *blk_len = NULL;
    int i, j, k, n, numblks, numtimes, ii, numgaps;
    PIO_Offset bsize, bsizeg, blklensum;
    PIO_Offset del_arr[arrlen - 1];
    PIO_Offset loc_arr[arrlen - 1];

    numblks = 0;
    numgaps = 0;
    numtimes = 0;

    for (i = 0; i < arrlen - 1; i++)
    {
        del_arr[i] = arr_in[i + 1] - arr_in[i];
        if (del_arr[i] != 1)
        {
            numtimes++;
            if ( i > 0 && del_arr[i - 1] > 1)
                return(1);
        }
    }

    numblks = numtimes + 1;
    if (numtimes == 0)
        n = numblks;
    else
        n = numtimes;

    bsize = (PIO_Offset)arrlen;
    if (numblks > 1)
    {
        PIO_Offset blk_len[numblks];
        PIO_Offset gaps[numtimes];
        if(numtimes > 0)
        {
            ii = 0;
            for (i = 0; i < arrlen - 1; i++)
                if (del_arr[i] > 1)
                    gaps[ii++] = del_arr[i] - 1;
            numgaps = ii;
        }

        j = 0;
        for (i = 0; i < n; i++)
            loc_arr[i] = 1;

        for (i = 0; i < arrlen - 1; i++)
            if(del_arr[i] != 1)
                loc_arr[j++] = i;

        blk_len[0] = loc_arr[0];
        blklensum = blk_len[0];
        for(i = 1; i < numblks - 1; i++)
        {
            blk_len[i] = loc_arr[i] - loc_arr[i - 1];
            blklensum += blk_len[i];
        }
        blk_len[numblks - 1] = arrlen - blklensum;

        bsize = lgcd_array(numblks, blk_len);
        if (numgaps > 0)
        {
            bsizeg = lgcd_array(numgaps, gaps);
            bsize = lgcd(bsize, bsizeg);
        }
        if(arr_in[0] > 0)
            bsize = lgcd(bsize, arr_in[0]);
    }
    return bsize;
}

/**
 * Compute start and count values for each io task.
 *
 * @param basetype
 * @param ndims
 * @param gdims
 * @param num_io_procs
 * @param myiorank
 * @param start
 * @param kount
 */
int CalcStartandCount(int basetype, int ndims, const int *gdims, int num_io_procs,
                      int myiorank, PIO_Offset *start, PIO_Offset *kount)
{
    int minbytes;
    int maxbytes;
    int minblocksize;
    int basesize;
    int use_io_procs;
    int i;
    long int p;
    long int pgdims;
    bool converged;
    int iorank;
    int ldims;
    int tiorank;
    int ioprocs;
    int tioprocs;
    int mystart[ndims], mykount[ndims];
    long int pknt;
    long int tpsize=0;

    minbytes = blocksize - 256;
    maxbytes  = blocksize + 256;

    switch (basetype)
    {
    case PIO_INT:
        basesize = sizeof(int);
        break;
    case PIO_REAL:
        basesize = sizeof(float);
        break;
    case PIO_DOUBLE:
        basesize = sizeof(double);
        break;
    default:
#ifndef TESTCALCDECOMP
        piodie("Invalid basetype ",__FILE__,__LINE__);
#endif
        break;
    }
    minblocksize = minbytes / basesize;

    pgdims = 1;
    for (i = 0; i < ndims; i++)
        pgdims *= (long int)gdims[i];
    p = pgdims;
    use_io_procs = max(1, min((int) ((float)p / (float)minblocksize + 0.5), num_io_procs));
    converged = 0;
    for (i = 0; i < ndims; i++)
    {
        mystart[i] = 0;
        mykount[i] = 0;
    }

    while (!converged)
    {
        for (iorank = 0; iorank < use_io_procs; iorank++)
        {
            for (i = 0; i < ndims; i++)
            {
                start[i] = 0;
                kount[i] = gdims[i];
            }
            ldims = ndims - 1;
            p = basesize;
            for (i = ndims - 1; i >= 0; i--)
            {
                p = p * gdims[i];
                if (p / use_io_procs > maxbytes)
                {
                    ldims = i;
                    break;
                }
            }

            if (gdims[ldims] < use_io_procs)
            {
                if (ldims > 0 && gdims[ldims-1] > use_io_procs)
                    ldims--;
                else
                    use_io_procs -= (use_io_procs % gdims[ldims]);
            }

            ioprocs = use_io_procs;
            tiorank = iorank;
#ifdef TESTCALCDECOMP
            if(myiorank==0)      printf("%d use_io_procs %d ldims %d\n",__LINE__,use_io_procs,ldims);
#endif
            for (i = 0; i <= ldims; i++)
            {
                if (gdims[i] >= ioprocs)
                {
                    computestartandcount(gdims[i], ioprocs, tiorank, start + i, kount + i);
#ifdef TESTCALCDECOMP
                    if(myiorank==0)      printf("%d tiorank %d i %d start %d count %d\n",__LINE__,tiorank,i,start[i],kount[i]);
#endif
                    if (start[i] + kount[i] > gdims[i] + 1)
                    {
#ifndef TESTCALCDECOMP
                        piodie("Start plus count exceeds dimension bound",__FILE__,__LINE__);
#endif
                    }
                }
                else if(gdims[i] > 1)
                {
                    tioprocs = gdims[i];
                    tiorank = (iorank * tioprocs) / ioprocs;
                    computestartandcount(gdims[i], tioprocs, tiorank, start + i, kount + i);
                    ioprocs = ioprocs / tioprocs;
                    tiorank  = iorank % ioprocs;
                }

            }
            
            if (myiorank == iorank)
            {
                for (i = 0; i < ndims; i++)
                {
                    mystart[i] = start[i];
                    mykount[i] = kount[i];
                }
            }
            pknt = 1;

            for(i = 0; i < ndims; i++)
                pknt *= kount[i];

            tpsize += pknt;

            if (tpsize == pgdims && use_io_procs == iorank + 1)
            {
                converged = true;
                break;
            }
            else if(tpsize >= pgdims)
            {
                break;
            }
        }

        if (!converged)
        {
#ifdef TESTCALCDECOMP
            printf("%d start %d %d count %d %d tpsize %ld\n",__LINE__,mystart[0],mystart[1],mykount[0],mykount[1],tpsize);
#endif
            tpsize = 0;
            use_io_procs--;
        }
    }

    if (myiorank < use_io_procs)
    {
        for (i = 0; i < ndims; i++)
        {
            start[i] = mystart[i];
            kount[i] = mykount[i];
        }
    }
    else
    {
        for (i = 0; i < ndims; i++)
        {
            start[i] = 0;
            kount[i] = 0;
        }
    }

    return use_io_procs;
}

#ifdef TESTCALCDECOMP
int main()
{
    int ndims=2;
    int gdims[2]={31,777602};
    int num_io_procs=24;
    bool converged=false;
    long int start[ndims], kount[ndims];
    int iorank, numaiotasks;
    long int tpsize;
    long int psize;
    long int pgdims;
    int scnt;

    pgdims=1;
    for(int i=0;i<ndims;i++)
        pgdims*=gdims[i];

    tpsize = 0;
    numaiotasks = 0;
    while (! converged){
        for(iorank=0;iorank<num_io_procs;iorank++){
            numaiotasks = CalcStartandCount(PIO_DOUBLE,ndims,gdims,num_io_procs, iorank, start,kount);
            if(iorank<numaiotasks)
                printf("iorank %d start %ld %ld count %ld %ld\n",iorank,start[0],start[1],kount[0],kount[1]);

            if(numaiotasks<0) return numaiotasks;

            psize=1;
            scnt=0;
            for(int i=0;i<ndims;i++){
                psize*=kount[i];
                scnt+=kount[i];
            }
            tpsize+=psize;

        }
        if(tpsize==pgdims)
            converged = true;
        else{
            printf("Failed to converge %ld %ld %d\n",tpsize,pgdims,num_io_procs);
            tpsize=0;
            num_io_procs--;
        }
    }




}
#endif
