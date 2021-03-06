#!/bin/sh

# Get/Generate the Dashboard Model
if [ $# -eq 0 ]; then
	model=Experimental
else
	model=$1
fi

module reset
module unload netcdf
module swap intel pgi/16.5
module load git/2.3.0
module load cmake/3.0.2
module load netcdf-mpi/4.4.1
module load pnetcdf/1.7.0

export CC=mpicc
export FC=mpif90

export PIO_DASHBOARD_ROOT=`pwd`/dashboard
export PIO_COMPILER_ID=PGI-`$CC --version | head -n 2 | tail -n 1 | cut -d' ' -f2`

if [ ! -d "$PIO_DASHBOARD_ROOT" ]; then
  mkdir "$PIO_DASHBOARD_ROOT"
fi
cd "$PIO_DASHBOARD_ROOT"

if [ ! -d src ]; then
  git clone --branch develop https://github.com/PARALLELIO/ParallelIO src
  cd src
else
  cd src
  git fetch origin
  git checkout develop
fi

ctest -S CTestScript.cmake,${model} -VV
