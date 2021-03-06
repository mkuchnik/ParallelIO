include (LibMPI)

#==============================================================================
#  GENERATE TARGET SOURCES
#==============================================================================

SET(GENERATED_SRCS pio_file_simple_tests.F90
  pio_init_finalize.F90
  pio_fail.F90
  pio_file_fail.F90
  ncdf_simple_tests.F90
  ncdf_get_put.F90
  ncdf_fail.F90
  pio_decomp_tests.F90
  pio_decomp_tests_1d.F90
  pio_decomp_tests_2d.F90
  pio_decomp_tests_3d.F90
  pio_decomp_frame_tests.F90
  pio_decomp_fillval.F90
  pio_iosystem_tests.F90
  pio_iosystem_tests2.F90
  pio_iosystem_tests3.F90)

foreach (SRC_FILE IN LISTS GENERATED_SRCS)
  add_custom_command (
    OUTPUT ${SRC_FILE}
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/util/pio_tf_f90gen.pl
    --annotate-source
    --out=${CMAKE_CURRENT_BINARY_DIR}/${SRC_FILE}
    ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FILE}.in
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FILE}.in)
endforeach ()

if ("${CMAKE_Fortran_COMPILER_ID}" STREQUAL "GNU")
  add_definitions(-ffree-line-length-none)
endif()

if (CMAKE_Fortran_COMPILER_ID STREQUAL "NAG")
  set ( CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -mismatch_all" )
  #    target_compile_options (gptl
  #        PRIVATE -mismatch_all)
endif ()

#==============================================================================
#  DEFINE THE TARGETS AND TESTS
#==============================================================================

# Test Timeout (4 min = 240 sec)
set (DEFAULT_TEST_TIMEOUT 240)

#===== pio_init_finalize =====
add_executable (pio_init_finalize EXCLUDE_FROM_ALL
  pio_init_finalize.F90
  ${CMAKE_CURRENT_SOURCE_DIR}/util/pio_tutil.F90)
target_link_libraries (pio_init_finalize piof)
add_dependencies (tests pio_init_finalize)

if (PIO_USE_MPISERIAL)
  add_test(NAME init_finialize_1_proc
    COMMAND pio_init_finalize)
  set_tests_properties(init_finialize_1_proc
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_test(NAME init_finialize_2_proc
    COMMAND pio_init_finalize)
  set_tests_properties(init_finialize_2_proc
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_test(NAME init_finalize_2_proc_with_args
    COMMAND pio_init_finalize --pio-tf-stride=2 --pio-tf-num-aggregators=2)
  set_tests_properties(init_finalize_2_proc_with_args
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
else ()
  add_mpi_test(init_finialize_1_proc
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_init_finalize
    NUMPROCS 1
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_mpi_test(init_finialize_2_proc
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_init_finalize
    NUMPROCS 2
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_mpi_test(init_finalize_2_proc_with_args
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_init_finalize
    ARGUMENTS --pio-tf-stride=2 --pio-tf-num-aggregators=2
    NUMPROCS 2
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
endif ()

#===== pio_file_simple_tests =====
add_executable (pio_file_simple_tests EXCLUDE_FROM_ALL
  pio_file_simple_tests.F90
  ${CMAKE_CURRENT_SOURCE_DIR}/util/pio_tutil.F90)
target_link_libraries (pio_file_simple_tests piof)
add_dependencies (tests pio_file_simple_tests)

if (PIO_USE_MPISERIAL)
  add_test(NAME pio_file_simple_tests
    COMMAND pio_file_simple_tests)
  set_tests_properties(pio_file_simple_tests
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
else ()
  add_mpi_test(pio_file_simple_tests
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_file_simple_tests
    NUMPROCS 2
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
endif ()

#===== pio_file_fail =====
add_executable (pio_file_fail EXCLUDE_FROM_ALL
  pio_file_fail.F90
  ${CMAKE_CURRENT_SOURCE_DIR}/util/pio_tutil.F90)
target_link_libraries (pio_file_fail piof)
if ("${CMAKE_Fortran_COMPILER_ID}" STREQUAL "GNU")
  target_compile_options (pio_init_finalize
    PRIVATE -ffree-line-length-none)
endif()
add_dependencies (tests pio_file_fail)

if (PIO_USE_MPISERIAL)
  add_test(NAME pio_file_fail
    COMMAND pio_file_fail)
  set_tests_properties(pio_file_fail
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
else ()
  add_mpi_test(pio_file_fail
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_file_fail
    NUMPROCS 2
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
endif ()

#===== ncdf_simple_tests =====
add_executable (ncdf_simple_tests EXCLUDE_FROM_ALL
  ncdf_simple_tests.F90
  ${CMAKE_CURRENT_SOURCE_DIR}/util/pio_tutil.F90)
target_link_libraries (ncdf_simple_tests piof)
add_dependencies (tests ncdf_simple_tests)

if (PIO_USE_MPISERIAL)
  add_test(NAME ncdf_simple_tests
    COMMAND ncdf_simple_tests)
  set_tests_properties(ncdf_simple_tests
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
else ()
  add_mpi_test(ncdf_simple_tests
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/ncdf_simple_tests
    NUMPROCS 2
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
endif ()

#===== ncdf_get_put =====
add_executable (ncdf_get_put EXCLUDE_FROM_ALL
  ncdf_get_put.F90
  ${CMAKE_CURRENT_SOURCE_DIR}/util/pio_tutil.F90)
target_link_libraries (ncdf_get_put piof)
add_dependencies (tests ncdf_get_put)

if (PIO_USE_MPISERIAL)
  add_test(NAME ncdf_get_put_1proc
    COMMAND ncdf_get_put)
  set_tests_properties(ncdf_get_put_1proc
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME ncdf_get_put_2proc
    COMMAND ncdf_get_put)
  set_tests_properties(ncdf_get_put_2proc
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
else ()
  add_mpi_test(ncdf_get_put_1proc
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/ncdf_get_put
    NUMPROCS 1
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(ncdf_get_put_2proc
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/ncdf_get_put
    NUMPROCS 2
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
endif ()

#===== ncdf_fail =====
add_executable (ncdf_fail EXCLUDE_FROM_ALL
  ncdf_fail.F90
  ${CMAKE_CURRENT_SOURCE_DIR}/util/pio_tutil.F90)
target_link_libraries (ncdf_fail piof)
add_dependencies (tests ncdf_fail)

if (PIO_USE_MPISERIAL)
  add_test(NAME ncdf_fail
    COMMAND ncdf_fail)
  set_tests_properties(ncdf_fail
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
else ()
  add_mpi_test(ncdf_fail
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/ncdf_fail
    NUMPROCS 2
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
endif ()

#===== pio_decomp_tests =====
add_executable (pio_decomp_tests EXCLUDE_FROM_ALL
  pio_decomp_tests.F90
  ${CMAKE_CURRENT_SOURCE_DIR}/util/pio_tutil.F90)
target_link_libraries (pio_decomp_tests piof)
add_dependencies (tests pio_decomp_tests)

if (PIO_USE_MPISERIAL)
  add_test(NAME pio_decomp_tests_1p
    COMMAND pio_decomp_tests)
  set_tests_properties(pio_decomp_tests_1p
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_2p
    COMMAND pio_decomp_tests)
  set_tests_properties(pio_decomp_tests_2p
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_3p
    COMMAND pio_decomp_tests)
  set_tests_properties(pio_decomp_tests_3p
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_test(NAME pio_decomp_tests_4p_1agg
    COMMAND pio_decomp_tests --pio-tf-num-aggregators=1)
  set_tests_properties(pio_decomp_tests_4p_1agg
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_4p_2agg
    COMMAND pio_decomp_tests --pio-tf-num-aggregators=2)
  set_tests_properties(pio_decomp_tests_4p_2agg
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_4p_3agg
    COMMAND pio_decomp_tests --pio-tf-num-aggregators=3)
  set_tests_properties(pio_decomp_tests_4p_3agg
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_test(NAME pio_decomp_tests_4p_1iop
    COMMAND pio_decomp_tests --pio-tf-num-io-tasks=1)
  set_tests_properties(pio_decomp_tests_4p_1iop
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_4p_2iop
    COMMAND pio_decomp_tests --pio-tf-num-io-tasks=2)
  set_tests_properties(pio_decomp_tests_4p_2iop
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_4p_3iop
    COMMAND pio_decomp_tests --pio-tf-num-io-tasks=3)
  set_tests_properties(pio_decomp_tests_4p_3iop
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_test(NAME pio_decomp_tests_4p_2iop_2str
    COMMAND pio_decomp_tests --pio-tf-num-io-tasks=2 --pio-tf-stride=2)
  set_tests_properties(pio_decomp_tests_4p_2iop_2str
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_4p_2iop_1agg
    COMMAND pio_decomp_tests --pio-tf-num-io-tasks=2 --pio-tf-num-aggregators=1)
  set_tests_properties(pio_decomp_tests_4p_2iop_1agg
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
else ()
  add_mpi_test(pio_decomp_tests_1p
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests
    NUMPROCS 1
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_2p
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests
    NUMPROCS 2
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_3p
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests
    NUMPROCS 3
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_mpi_test(pio_decomp_tests_4p_1agg
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests
    ARGUMENTS --pio-tf-num-aggregators=1
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_4p_2agg
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests
    ARGUMENTS --pio-tf-num-aggregators=2
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_4p_3agg
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests
    ARGUMENTS --pio-tf-num-aggregators=3
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_mpi_test(pio_decomp_tests_4p_1iop
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests
    ARGUMENTS --pio-tf-num-io-tasks=1
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_4p_2iop
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests
    ARGUMENTS --pio-tf-num-io-tasks=2
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_4p_3iop
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests
    ARGUMENTS --pio-tf-num-io-tasks=3
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_mpi_test(pio_decomp_tests_4p_2iop_2str
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests
    ARGUMENTS --pio-tf-num-io-tasks=2 --pio-tf-stride=2
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_4p_2iop_1agg
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests
    ARGUMENTS --pio-tf-num-io-tasks=2 --pio-tf-num-aggregators=1
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
endif ()

#===== pio_decomp_tests_1d =====
add_executable (pio_decomp_tests_1d EXCLUDE_FROM_ALL
  pio_decomp_tests_1d.F90
  ${CMAKE_CURRENT_SOURCE_DIR}/util/pio_tutil.F90)
target_link_libraries (pio_decomp_tests_1d piof)
add_dependencies (tests pio_decomp_tests_1d)

if (PIO_USE_MPISERIAL)
  add_test(NAME pio_decomp_tests_1d_1p
    COMMAND pio_decomp_tests_1d)
  set_tests_properties(pio_decomp_tests_1d_1p
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_1d_2p
    COMMAND pio_decomp_tests_1d)
  set_tests_properties(pio_decomp_tests_1d_2p
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_1d_3p
    COMMAND pio_decomp_tests_1d)
  set_tests_properties(pio_decomp_tests_1d_3p
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_test(NAME pio_decomp_tests_1d_4p_1agg
    COMMAND pio_decomp_tests_1d --pio-tf-num-aggregators=1)
  set_tests_properties(pio_decomp_tests_1d_4p_1agg
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_1d_4p_2agg
    COMMAND pio_decomp_tests_1d --pio-tf-num-aggregators=2)
  set_tests_properties(pio_decomp_tests_1d_4p_2agg
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_1d_4p_3agg
    COMMAND pio_decomp_tests_1d --pio-tf-num-aggregators=3)
  set_tests_properties(pio_decomp_tests_1d_4p_3agg
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_test(NAME pio_decomp_tests_1d_4p_1iop
    COMMAND pio_decomp_tests_1d --pio-tf-num-io-tasks=1)
  set_tests_properties(pio_decomp_tests_1d_4p_1iop
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_1d_4p_2iop
    COMMAND pio_decomp_tests_1d --pio-tf-num-io-tasks=2)
  set_tests_properties(pio_decomp_tests_1d_4p_2iop
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_1d_4p_3iop
    COMMAND pio_decomp_tests_1d --pio-tf-num-io-tasks=3)
  set_tests_properties(pio_decomp_tests_1d_4p_3iop
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_test(NAME pio_decomp_tests_1d_4p_2iop_2str
    COMMAND pio_decomp_tests_1d --pio-tf-num-io-tasks=2 --pio-tf-stride=2)
  set_tests_properties(pio_decomp_tests_1d_4p_2iop_2str
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_1d_4p_2iop_1agg
    COMMAND pio_decomp_tests_1d --pio-tf-num-io-tasks=2 --pio-tf-num-aggregators=1)
  set_tests_properties(pio_decomp_tests_1d_4p_2iop_1agg
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
else ()
  add_mpi_test(pio_decomp_tests_1d_1p
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_1d
    NUMPROCS 1
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_1d_2p
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_1d
    NUMPROCS 2
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_1d_3p
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_1d
    NUMPROCS 3
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_mpi_test(pio_decomp_tests_1d_4p_1agg
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_1d
    ARGUMENTS --pio-tf-num-aggregators=1
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_1d_4p_2agg
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_1d
    ARGUMENTS --pio-tf-num-aggregators=2
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_1d_4p_3agg
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_1d
    ARGUMENTS --pio-tf-num-aggregators=3
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_mpi_test(pio_decomp_tests_1d_4p_1iop
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_1d
    ARGUMENTS --pio-tf-num-io-tasks=1
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_1d_4p_2iop
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_1d
    ARGUMENTS --pio-tf-num-io-tasks=2
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_1d_4p_3iop
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_1d
    ARGUMENTS --pio-tf-num-io-tasks=3
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_mpi_test(pio_decomp_tests_1d_4p_2iop_2str
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_1d
    ARGUMENTS --pio-tf-num-io-tasks=2 --pio-tf-stride=2
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_1d_4p_2iop_1agg
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_1d
    ARGUMENTS --pio-tf-num-io-tasks=2 --pio-tf-num-aggregators=1
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
endif ()

#===== pio_decomp_tests_2d =====
add_executable (pio_decomp_tests_2d EXCLUDE_FROM_ALL
  pio_decomp_tests_2d.F90
  ${CMAKE_CURRENT_SOURCE_DIR}/util/pio_tutil.F90)
target_link_libraries (pio_decomp_tests_2d piof)
add_dependencies (tests pio_decomp_tests_2d)

if (PIO_USE_MPISERIAL)
  add_test(NAME pio_decomp_tests_2d_1p
    COMMAND pio_decomp_tests_2d)
  set_tests_properties(pio_decomp_tests_2d_1p
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_2d_2p
    COMMAND pio_decomp_tests_2d)
  set_tests_properties(pio_decomp_tests_2d_2p
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_2d_3p
    COMMAND pio_decomp_tests_2d)
  set_tests_properties(pio_decomp_tests_2d_3p
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_test(NAME pio_decomp_tests_2d_4p_1agg
    COMMAND pio_decomp_tests_2d --pio-tf-num-aggregators=1)
  set_tests_properties(pio_decomp_tests_2d_4p_1agg
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_2d_4p_2agg
    COMMAND pio_decomp_tests_2d --pio-tf-num-aggregators=2)
  set_tests_properties(pio_decomp_tests_2d_4p_2agg
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_2d_4p_3agg
    COMMAND pio_decomp_tests_2d --pio-tf-num-aggregators=3)
  set_tests_properties(pio_decomp_tests_2d_4p_3agg
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_test(NAME pio_decomp_tests_2d_4p_1iop
    COMMAND pio_decomp_tests_2d --pio-tf-num-io-tasks=1)
  set_tests_properties(pio_decomp_tests_2d_4p_1iop
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_2d_4p_2iop
    COMMAND pio_decomp_tests_2d --pio-tf-num-io-tasks=2)
  set_tests_properties(pio_decomp_tests_2d_4p_2iop
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_2d_4p_3iop
    COMMAND pio_decomp_tests_2d --pio-tf-num-io-tasks=3)
  set_tests_properties(pio_decomp_tests_2d_4p_3iop
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_test(NAME pio_decomp_tests_2d_4p_2iop_2str
    COMMAND pio_decomp_tests_2d --pio-tf-num-io-tasks=2 --pio-tf-stride=2)
  set_tests_properties(pio_decomp_tests_2d_4p_2iop_2str
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_2d_4p_2iop_1agg
    COMMAND pio_decomp_tests_2d --pio-tf-num-io-tasks=2 --pio-tf-num-aggregators=1)
  set_tests_properties(pio_decomp_tests_2d_4p_2iop_1agg
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
else ()
  add_mpi_test(pio_decomp_tests_2d_1p
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_2d
    NUMPROCS 1
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_2d_2p
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_2d
    NUMPROCS 2
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_2d_3p
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_2d
    NUMPROCS 3
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_mpi_test(pio_decomp_tests_2d_4p_1agg
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_2d
    ARGUMENTS --pio-tf-num-aggregators=1
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_2d_4p_2agg
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_2d
    ARGUMENTS --pio-tf-num-aggregators=2
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_2d_4p_3agg
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_2d
    ARGUMENTS --pio-tf-num-aggregators=3
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_mpi_test(pio_decomp_tests_2d_4p_1iop
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_2d
    ARGUMENTS --pio-tf-num-io-tasks=1
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_2d_4p_2iop
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_2d
    ARGUMENTS --pio-tf-num-io-tasks=2
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_2d_4p_3iop
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_2d
    ARGUMENTS --pio-tf-num-io-tasks=3
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_mpi_test(pio_decomp_tests_2d_4p_2iop_2str
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_2d
    ARGUMENTS --pio-tf-num-io-tasks=2 --pio-tf-stride=2
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_2d_4p_2iop_1agg
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_2d
    ARGUMENTS --pio-tf-num-io-tasks=2 --pio-tf-num-aggregators=1
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
endif ()

#===== pio_decomp_tests_3d =====
add_executable (pio_decomp_tests_3d EXCLUDE_FROM_ALL
  pio_decomp_tests_3d.F90
  ${CMAKE_CURRENT_SOURCE_DIR}/util/pio_tutil.F90)
target_link_libraries (pio_decomp_tests_3d piof)
add_dependencies (tests pio_decomp_tests_3d)

if (PIO_USE_MPISERIAL)
  add_test(NAME pio_decomp_tests_3d_1p
    COMMAND pio_decomp_tests_3d)
  set_tests_properties(pio_decomp_tests_3d_1p
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_3d_2p
    COMMAND pio_decomp_tests_3d)
  set_tests_properties(pio_decomp_tests_3d_2p
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_3d_3p
    COMMAND pio_decomp_tests_3d)
  set_tests_properties(pio_decomp_tests_3d_3p
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_test(NAME pio_decomp_tests_3d_4p_1agg
    COMMAND pio_decomp_tests_3d --pio-tf-num-aggregators=1)
  set_tests_properties(pio_decomp_tests_3d_4p_1agg
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_3d_4p_2agg
    COMMAND pio_decomp_tests_3d --pio-tf-num-aggregators=2)
  set_tests_properties(pio_decomp_tests_3d_4p_2agg
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_3d_4p_3agg
    COMMAND pio_decomp_tests_3d --pio-tf-num-aggregators=3)
  set_tests_properties(pio_decomp_tests_3d_4p_3agg
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_test(NAME pio_decomp_tests_3d_4p_1iop
    COMMAND pio_decomp_tests_3d --pio-tf-num-io-tasks=1)
  set_tests_properties(pio_decomp_tests_3d_4p_1iop
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_3d_4p_2iop
    COMMAND pio_decomp_tests_3d --pio-tf-num-io-tasks=2)
  set_tests_properties(pio_decomp_tests_3d_4p_2iop
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_3d_4p_3iop
    COMMAND pio_decomp_tests_3d --pio-tf-num-io-tasks=3)
  set_tests_properties(pio_decomp_tests_3d_4p_3iop
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_test(NAME pio_decomp_tests_3d_4p_2iop_2str
    COMMAND pio_decomp_tests_3d --pio-tf-num-io-tasks=2 --pio-tf-stride=2)
  set_tests_properties(pio_decomp_tests_3d_4p_2iop_2str
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_test(NAME pio_decomp_tests_3d_4p_2iop_1agg
    COMMAND pio_decomp_tests_3d --pio-tf-num-io-tasks=2 --pio-tf-num-aggregators=1)
  set_tests_properties(pio_decomp_tests_3d_4p_2iop_1agg
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
else ()
  add_mpi_test(pio_decomp_tests_3d_1p
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_3d
    NUMPROCS 1
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_3d_2p
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_3d
    NUMPROCS 2
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_3d_3p
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_3d
    NUMPROCS 3
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_mpi_test(pio_decomp_tests_3d_4p_1agg
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_3d
    ARGUMENTS --pio-tf-num-aggregators=1
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_3d_4p_2agg
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_3d
    ARGUMENTS --pio-tf-num-aggregators=2
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_3d_4p_3agg
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_3d
    ARGUMENTS --pio-tf-num-aggregators=3
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_mpi_test(pio_decomp_tests_3d_4p_1iop
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_3d
    ARGUMENTS --pio-tf-num-io-tasks=1
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_3d_4p_2iop
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_3d
    ARGUMENTS --pio-tf-num-io-tasks=2
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_3d_4p_3iop
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_3d
    ARGUMENTS --pio-tf-num-io-tasks=3
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})

  add_mpi_test(pio_decomp_tests_3d_4p_2iop_2str
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_3d
    ARGUMENTS --pio-tf-num-io-tasks=2 --pio-tf-stride=2
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(pio_decomp_tests_3d_4p_2iop_1agg
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_tests_3d
    ARGUMENTS --pio-tf-num-io-tasks=2 --pio-tf-num-aggregators=1
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
endif ()

#===== pio_decomp_frame_tests =====
add_executable (pio_decomp_frame_tests EXCLUDE_FROM_ALL
  pio_decomp_frame_tests.F90
  ${CMAKE_CURRENT_SOURCE_DIR}/util/pio_tutil.F90)
target_link_libraries (pio_decomp_frame_tests piof)
add_dependencies (tests pio_decomp_frame_tests)

if (PIO_USE_MPISERIAL)
  add_test(NAME pio_decomp_frame_tests
    COMMAND pio_decomp_frame_tests)
  set_tests_properties(pio_decomp_frame_tests
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
else ()
  add_mpi_test(pio_decomp_frame_tests
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_frame_tests
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
endif ()

#===== pio_decomp_fillval =====
add_executable (pio_decomp_fillval EXCLUDE_FROM_ALL
  pio_decomp_fillval.F90
  ${CMAKE_CURRENT_SOURCE_DIR}/util/pio_tutil.F90)
target_link_libraries (pio_decomp_fillval piof)
add_dependencies (tests pio_decomp_fillval)

if (PIO_USE_MPISERIAL)
  add_test(NAME pio_decomp_fillval
    COMMAND pio_decomp_fillval)
  set_tests_properties(pio_decomp_fillval
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
else ()
  add_mpi_test(pio_decomp_fillval
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_decomp_fillval
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
endif ()

#===== pio_iosystems_test =====
add_executable (pio_iosystem_tests EXCLUDE_FROM_ALL
  pio_iosystem_tests.F90
  ${CMAKE_CURRENT_SOURCE_DIR}/util/pio_tutil.F90)
target_link_libraries (pio_iosystem_tests piof)
add_dependencies (tests pio_iosystem_tests)

if (PIO_USE_MPISERIAL)
  add_test(NAME pio_iosystem_tests
    COMMAND pio_iosystem_tests)
  set_tests_properties(pio_iosystem_tests
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
else ()
  add_mpi_test(pio_iosystem_tests
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_iosystem_tests
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
endif ()


#===== pio_iosystems_test2 =====
add_executable (pio_iosystem_tests2 EXCLUDE_FROM_ALL
  pio_iosystem_tests2.F90
  ${CMAKE_CURRENT_SOURCE_DIR}/util/pio_tutil.F90)
target_link_libraries (pio_iosystem_tests2 piof)
add_dependencies (tests pio_iosystem_tests2)

if (PIO_USE_MPISERIAL)
  add_test(NAME pio_iosystem_tests2
    COMMAND pio_iosystem_tests2)
  set_tests_properties(pio_iosystem_tests2
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
else ()
  add_mpi_test(pio_iosystem_tests2
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_iosystem_tests2
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
endif ()

#===== pio_iosystems_test3 =====
add_executable (pio_iosystem_tests3 EXCLUDE_FROM_ALL
  pio_iosystem_tests3.F90
  ${CMAKE_CURRENT_SOURCE_DIR}/util/pio_tutil.F90)
target_link_libraries (pio_iosystem_tests3 piof)
add_dependencies (tests pio_iosystem_tests3)

if (PIO_USE_MPISERIAL)
  add_test(NAME pio_iosystem_tests3
    COMMAND pio_iosystem_tests3)
  set_tests_properties(pio_iosystem_tests3
    PROPERTIES TIMEOUT ${DEFAULT_TEST_TIMEOUT})
else ()
  add_mpi_test(pio_iosystem_tests3
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/pio_iosystem_tests3
    NUMPROCS 4
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
endif ()
