#include <pio.h>
#include <pio_internal.h>
#define DEF_P2P_MAXREQ 64

int gindex_to_coord(const PIO_Offset gindex, const PIO_Offset gstride[], const int ndim, PIO_Offset *gcoord)
{
  PIO_Offset tempindex;

  tempindex = gindex;
  for(int i=ndim-1;i>0;i--){
    gcoord[i] = tempindex/gstride[i];
    tempindex -= gcoord[i]*gstride[i];
  }
  gcoord[0] = tempindex;
  return PIO_NOERR;
}

PIO_Offset lcoord_to_lindex(const int ndims, const PIO_Offset lcoord[], const PIO_Offset start[], const PIO_Offset count[])
{
  PIO_Offset lindex=0;
  PIO_Offset stride=1;

  for(int i=0; i<ndims; i++){
    lindex += lcoord[i]*stride;
    stride = stride*count[i];
  }
  return lindex;

}

int create_mpi_datatypes(const MPI_Datatype basetype,const int msgcnt,PIO_Offset mindex[],const int mcount[],MPI_Datatype mtype[])
{
  PIO_Offset bsizeT[msgcnt];
  int pos;
  int ii;
  PIO_Offset i8blocksize;
  MPI_Datatype newtype;
  int blocksize;
  bool freenewtype=true;

  bsizeT[0]=0;
  mtype[0] = MPI_DATATYPE_NULL;
  pos = 0;
  ii = 0;
  if(msgcnt>0){
    for(int i=0;i<msgcnt;i++){
      //      printf("mcount %d mindex %ld\n",mcount[i],mindex[i]);
      if(mcount[i]>0){
	bsizeT[ii] = GCDblocksize(mcount[i], mindex+pos);
	//		for(int j=0;j<mcount[i];j++)
	//		  printf(" %d ",mindex[pos+j]);
	//		printf("\n bsizet[%d] %ld\n",ii,bsizeT[ii]);
	ii++;
	pos+=mcount[i];
      }
    }
    blocksize = (int) lgcd_array(ii ,bsizeT);

    CheckMPIReturn(MPI_Type_contiguous(blocksize, basetype, &newtype),__FILE__,__LINE__);
    CheckMPIReturn(MPI_Type_commit(&newtype), __FILE__,__LINE__);
    
    //    printf("%d blocksize %d basetype %d newtype %d %d %d \n",msgcnt,blocksize,basetype,newtype,mindex[0],mcount[0]);


    pos = 0;
    for(int i=0;i< msgcnt; i++){
      if(mcount[i]>0){
	int len = mcount[i]/blocksize;
	if(len>1){
	  int displace[len];
	  if(blocksize==1)
	    for(int j=0;j<len;j++)
	      displace[j] = (int) (mindex+pos)[j];
	  else
	    for(int j=0;j<len;j++)
	      (mindex+pos)[j]++;

	  CheckMPIReturn(MPI_Type_create_indexed_block(len, 1, displace, newtype, mtype+i),__FILE__,__LINE__);
	  CheckMPIReturn(MPI_Type_commit(mtype+i), __FILE__,__LINE__);
	}else{
	  mtype[i] = newtype;
	  freenewtype= false;
	}
	pos+=mcount[i];

      }
    }
    if(freenewtype) CheckMPIReturn(MPI_Type_free(&newtype),__FILE__,__LINE__);
  }
  return PIO_NOERR;

}


int define_iodesc_datatypes(iosystem_desc_t *ios, io_desc_t *iodesc)
{
  if(ios->ioproc){
    if(iodesc->rtype==NULL){
      iodesc->rtype = (MPI_Datatype *) malloc(max(1,iodesc->nrecvs)*sizeof(MPI_Datatype));
      create_mpi_datatypes(iodesc->basetype, iodesc->nrecvs, iodesc->rindex, iodesc->rcount, iodesc->rtype);
    }
  }
  if(iodesc->stype==NULL){
    iodesc->stype = (MPI_Datatype *) malloc(ios->num_iotasks*sizeof(MPI_Datatype));
    create_mpi_datatypes(iodesc->basetype, ios->num_iotasks, iodesc->sindex, iodesc->scount, iodesc->stype);
  }
  return PIO_NOERR;

}




int compute_counts(iosystem_desc_t *ios, io_desc_t *iodesc, const int dest_ioproc[], 
		   const PIO_Offset dest_ioindex[])
{
  int niotasks = ios->num_iotasks;
  int ncomptasks = ios->num_comptasks;
  int i;
  int iorank;
  MPI_Datatype sr_types[ncomptasks];
  int send_counts[ncomptasks];
  int send_displs[ncomptasks];
  int recv_counts[ncomptasks];
  int recv_displs[ncomptasks];
  int *recv_buf;
  int *rcount;
  int *rfrom;
  int nrecvs;
  int rbuf_size;
  int pio_maxreq;  
  int ierr;
  int io_comprank;
  MPI_Datatype dtype;
  int ioindex;
  int tsize, tsizei, tsizel;
  int ndof= iodesc->ndof;


  pio_maxreq = DEF_P2P_MAXREQ;

  iodesc->scount = (int *) calloc(niotasks,sizeof(int));

  // iodesc->scount is the amount of data sent to each task from the current task
  for(i=0;i<ndof; i++){
    iorank = dest_ioproc[i];
    if(iorank != -1){
      (iodesc->scount[iorank])++;
    }
  }

  //  for(i=0;i<niotasks;i++)
  //   printf("iodesc->scount = %d\n",iodesc->scount[i]);

  for(i=0;i<ncomptasks;i++){
    send_counts[i] = 0;
    send_displs[i] = 0;
    recv_counts[i] = 0;
    recv_displs[i] = 0;
    sr_types[i] = MPI_INT;
  }

  for(i=0;i<niotasks;i++){
    int io_comprank = ios->ioranks[i];
    send_counts[io_comprank] = 1;
    send_displs[io_comprank] = i*sizeof(int);
  }


  if(ios->ioproc){
    recv_buf = (int *) malloc(ncomptasks * sizeof(int));
    for(i=0;i<ncomptasks;i++){
      recv_buf[i] = 0;
      recv_counts[i] = 1;
      recv_displs[i] = i*sizeof(int);
    }
    rbuf_size = ncomptasks;
  }else{
    recv_buf = (int *) malloc(sizeof(int));
     rbuf_size = 1;
    recv_buf[0]=0;
  }

  // Share the iodesc->scount from each compute task to all io tasks
  ierr = pio_swapm( iodesc->scount, send_counts, send_displs, sr_types, 
                    recv_buf,  recv_counts, recv_displs, sr_types,
		    ios->union_comm, false, false, pio_maxreq);


  nrecvs = 0;
  if(ios->ioproc){
    //    printf("recv_buf = ");
    for(i=0;i<ncomptasks; i++){
      //   printf(" %d ",recv_buf[i]);
      if(recv_buf[i] != 0)
	nrecvs++;
    }
    // printf("\n");

    iodesc->rcount = (int *) calloc(max(1,nrecvs),sizeof(int));
    rcount = iodesc->rcount;
    iodesc->rfrom = (int *) calloc(max(1,nrecvs),sizeof(int));
    
    rfrom = iodesc->rfrom;
    nrecvs = 0;
    for(i=0;i<ncomptasks; i++){
      if(recv_buf[i] != 0){
	rcount[nrecvs] = recv_buf[i];
	rfrom[nrecvs] = i;
	nrecvs++;
      }

    }
  }else{
    rcount = (int *) malloc(sizeof(int));
    rcount[0]=0;
  }

  iodesc->nrecvs = nrecvs;
  iodesc->sindex = (PIO_Offset *) calloc(ndof,sizeof(PIO_Offset));
  PIO_Offset s2rindex[ndof];

  int tempcount[niotasks];
  int spos[niotasks];

  spos[0]=0;
  tempcount[0]=0;
  for(i=1;i<niotasks;i++){
    spos[i] = spos[i-1] + iodesc->scount[i-1];
    tempcount[i]=0;
  }
  for(i=0;i<ndof;i++){
    iorank =dest_ioproc[i]; 
    ioindex = dest_ioindex[i];
    if(iorank > -1){
      iodesc->sindex[spos[iorank]+tempcount[iorank]] = i;
      s2rindex[spos[iorank]+tempcount[iorank]] = ioindex;
      (tempcount[iorank])++;
    }
  }

  for(i=0;i<ncomptasks;i++){
    send_counts[i] = 0;
    send_displs[i]  = 0;
    recv_counts[i] = 0;
    recv_displs[i]   =0;
  }
  for(i=0;i<niotasks;i++){
    io_comprank = ios->ioranks[i];
    send_counts[io_comprank] = iodesc->scount[i];
    send_displs[io_comprank]  = spos[i] ;
  }

  MPI_Type_size(MPI_LONG_LONG, &tsizel);
  MPI_Type_size(MPI_INT, &tsizei);

  if(sizeof(PIO_Offset) == tsizei){
    dtype = MPI_INT;
    tsize = tsizei;
  }else if(sizeof(PIO_Offset) == tsizel){
    dtype = MPI_LONG;
    tsize = tsizel;
  }

  for(i=0; i<ncomptasks; i++){
    sr_types[i] = dtype;
  }

  if(ios->ioproc){
    rbuf_size=max(1,iodesc->llen);
    
    for(i=0;i<nrecvs;i++)
      recv_counts[rfrom[i]] = rcount[i];
    recv_displs[0] = 0;
    for(i=1;i<nrecvs;i++)
      recv_displs[rfrom[i]] = recv_displs[rfrom[i-1]]+rcount[i-1]*tsize;
  }else{
    rbuf_size=1;
  }
  iodesc->rindex = (PIO_Offset *) calloc(rbuf_size,sizeof(PIO_Offset));

  // s2rindex is the list of indeces on each compute task

  ierr = pio_swapm( s2rindex, send_counts, send_displs, sr_types, 
		    iodesc->rindex, recv_counts, recv_displs, sr_types,
		    ios->union_comm, true, false,  MAX_GATHER_BLOCK_SIZE);

  //  rindex is an array of the indices of the data to be sent from
  //  this io task to each compute task. 

  //  for(i=0;i<rbuf_size;i++)
  //   printf("rindex[%d] %ld\n",i,rindex[i]);
  
  //  MPI_Abort(MPI_COMM_WORLD,0);

  iodesc->rtype = NULL;
  iodesc->stype = NULL;

  return ierr;

}

int box_rearrange_comp2io(iosystem_desc_t *ios, io_desc_t *iodesc, void *sbuf,
			  void *rbuf, const int comm_option, const int fc_options)
{

  bool handshake=true;
  bool isend = false;
  int maxreq = MAX_GATHER_BLOCK_SIZE;
  int nprocs = ios->num_comptasks;
  int *scount = iodesc->scount;
  int sendcounts[nprocs];
  int recvcounts[nprocs];
  int sdispls[nprocs];
  int rdispls[nprocs];
  MPI_Datatype sendtypes[nprocs];
  MPI_Datatype recvtypes[nprocs];
  int i, tsize;


  define_iodesc_datatypes(ios, iodesc);


  for(i=0;i<nprocs;i++){
    sendcounts[i] = 0;
    recvcounts[i] = 0; 
    sdispls[i] = 0; 
    rdispls[i] = 0;
    recvtypes[ i ] = MPI_DATATYPE_NULL;
    sendtypes[ i ] =  MPI_DATATYPE_NULL;
  }


  if(ios->ioproc && iodesc->nrecvs>0){
    recvcounts[ iodesc->rfrom[0] ] = 1;
    recvtypes[ iodesc->rfrom[0] ] = iodesc->rtype[0];
    rdispls[ iodesc->rfrom[0] ] = 0;
    for( i=1;i<iodesc->nrecvs;i++){
      recvcounts[ iodesc->rfrom[i] ] = 1;
      recvtypes[ iodesc->rfrom[i] ] = iodesc->rtype[i];
      MPI_Type_size(iodesc->rtype[i-1],&tsize);
      rdispls[ iodesc->rfrom[i] ] = rdispls[ iodesc->rfrom[i-1] ] + tsize;
    }
  }else{
    for( i=0;i<iodesc->nrecvs;i++){
      recvcounts[ iodesc->rfrom[i] ] = 0;
    }
  }  

  for( i=0;i<ios->num_iotasks; i++){
    int io_comprank = ios->ioranks[i];
    if(scount[i] > 0) {
      sendcounts[io_comprank]=1;
      sendtypes[io_comprank]=iodesc->stype[i];
      MPI_Type_size(iodesc->stype[i],&tsize);
      sdispls[io_comprank] = i*tsize;
    }else{
      sendcounts[io_comprank]=0;
    }
  }      

  // Data in sbuf on the compute nodes is sent to rbuf on the ionodes

  MPI_Barrier( ios->union_comm);
  pio_swapm( sbuf,  sendcounts, sdispls, sendtypes,
	     rbuf, recvcounts, rdispls, recvtypes, 
	     ios->union_comm, handshake, isend, 1); //MAX_GATHER_BLOCK_SIZE);


  return PIO_NOERR;
}

int box_rearrange_io2comp(iosystem_desc_t *ios, io_desc_t *iodesc, void *sbuf,
			  void *rbuf, const int comm_option, const int fc_options)
{
  

  bool handshake=true;
  bool isend = false;
  int maxreq = MAX_GATHER_BLOCK_SIZE;
  int nprocs = ios->num_comptasks;
  int *scount = iodesc->scount;
  int sendcounts[nprocs];
  int recvcounts[nprocs];
  int sdispls[nprocs];
  int rdispls[nprocs];
  MPI_Datatype sendtypes[nprocs];
  MPI_Datatype recvtypes[nprocs];
  int i, tsize;
  

  define_iodesc_datatypes(ios, iodesc);

  for( i=0;i< nprocs;i++){
    sendcounts[i] = 0;
    recvcounts[i] = 0; 
    sdispls[i] = 0; 
    rdispls[i] = 0;
    sendtypes[ i ] = MPI_DATATYPE_NULL;
    recvtypes[ i ] = MPI_DATATYPE_NULL;
  }
  if(ios->ioproc){
    for( i=0;i< iodesc->nrecvs;i++){
      sendcounts[ iodesc->rfrom[i] ] = 1;
      sendtypes[ iodesc->rfrom[i] ] = iodesc->rtype[i];
      MPI_Type_size(iodesc->rtype[i], &tsize);
      sdispls[ iodesc->rfrom[i] ] = i*tsize;
    }
  }
    

  for( i=0;i<ios->num_iotasks; i++){
    int io_comprank = ios->ioranks[i];
    if(scount[i] > 0) {
      recvcounts[io_comprank]=1;
      recvtypes[io_comprank]=iodesc->stype[i];
      MPI_Type_size(iodesc->stype[i],&tsize);
      rdispls[ io_comprank ] = i*tsize;
    }
  } 
  //
  // Data in sbuf on the ionodes is sent to rbuf on the compute nodes
  //
  pio_swapm( sbuf,  sendcounts, sdispls, sendtypes,
	     rbuf, recvcounts, rdispls, recvtypes, 
	     ios->union_comm, handshake,isend,maxreq);


  return PIO_NOERR;

}



int box_rearrange_create(iosystem_desc_t *ios,const int maplen, const PIO_Offset compmap[], const int gsize[],
			 const int ndims, const int nioproc, io_desc_t *iodesc)
{
  int ierr=PIO_NOERR;
  int nprocs = ios->num_comptasks;
  int nioprocs = ios->num_iotasks;
  PIO_Offset gstride[ndims];
  PIO_Offset iomap;
  PIO_Offset start[ndims], count[ndims];
  int tsizei, tsizel, tsize, i, j, k, llen;
  MPI_Datatype dtype;
  int dest_ioproc[maplen];
  PIO_Offset dest_ioindex[maplen];

  iodesc->ndof = maplen;
  gstride[0]=1;
  for(int i=1;i<ndims; i++)
    gstride[i]=gstride[i-1]*gsize[i-1];

  MPI_Type_size(MPI_LONG_LONG, &tsizel);
  MPI_Type_size(MPI_INT, &tsizei);

  if(sizeof(PIO_Offset) == tsizei){
    dtype = MPI_INT;
    tsize = tsizei;
  }else if(sizeof(PIO_Offset) == tsizel){
    dtype = MPI_LONG;
    tsize = tsizel;
  }
  int sndlths[nprocs]; 
  int sdispls[nprocs];
  int recvlths[nprocs];
  int rdispls[nprocs];
  MPI_Datatype dtypes[nprocs];


  for(i=0; i< maplen; i++){
    dest_ioproc[i] = -1;
    dest_ioindex[i] = 0;
  }
  for(i=0;i<nprocs;i++){
    sndlths[i] = 0;
    sdispls[i] = 0;
    recvlths[i] = 0;
    rdispls[i] = 0;
    dtypes[i] = dtype;
  }
  if(ios->ioproc){
    for( i=0;i<nprocs;i++){
      sndlths[ i ] = 1;
    }
  }
  for( i=0;i<nioprocs; i++){
    int io_comprank = ios->ioranks[i];
    recvlths[ io_comprank ] = 1;
    rdispls[ io_comprank ] = i*tsize;
  }      
  PIO_Offset iomaplen[nioprocs];
  //  The length of each iomap
  pio_swapm(&(iodesc->llen), sndlths, sdispls, dtypes,
	    iomaplen, recvlths, rdispls, dtypes, 	
	    ios->union_comm, false, false, MAX_GATHER_BLOCK_SIZE);


  for(i=0; i<nioprocs; i++){
    if(iomaplen[i]>0){
      int io_comprank = ios->ioranks[i];
      for( j=0; j<nprocs ; j++){
	sndlths[ j ] = 0;
	sdispls[ j ] = 0;
	rdispls[ j ] = 0;
	recvlths[ j ] = 0;
	if(ios->union_rank == io_comprank)
	  sndlths[ j ] = ndims;
      }
      recvlths[ io_comprank ] = ndims;
      
      // The count from iotask i is sent to all compute tasks
      pio_swapm(iodesc->count,  sndlths, sdispls, dtypes,
		count, recvlths, rdispls, dtypes, 
		ios->union_comm, false, false, MAX_GATHER_BLOCK_SIZE);
      
      // The start from iotask i is sent to all compute tasks
      pio_swapm(iodesc->start,  sndlths, sdispls, dtypes,
		start, recvlths, rdispls, dtypes, 
		ios->union_comm, false, false, MAX_GATHER_BLOCK_SIZE);
      for(k=0;k<maplen;k++){
	PIO_Offset gcoord[ndims], lcoord[ndims];
	bool found=true;
	ierr = gindex_to_coord(compmap[k]-1, gstride, ndims, gcoord);
	for(j=0;j<ndims;j++){
	  if(gcoord[j] >= start[j] && gcoord[j] < start[j]+count[j]){
	    lcoord[j] = gcoord[j] - start[j];
	  }else{
	    found = false;
	    break;
	  }
	}
	if(found){
	  dest_ioindex[k] = lcoord_to_lindex(ndims, lcoord, start, count);
	  dest_ioproc[k] = i;
	}
      }
    }
  }
  for(k=0; k<maplen; k++){
    if(dest_ioproc[k] == -1 && compmap[k]>=0){
      fprintf(stderr,"No destination found for compmap[%d] = %ld\n",k,compmap[k]);
      MPI_Abort(MPI_COMM_WORLD,0);
    }
  }

  compute_counts(ios, iodesc, dest_ioproc, dest_ioindex);

  return PIO_NOERR;
}
