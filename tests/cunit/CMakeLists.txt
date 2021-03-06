include (LibMPI)

include_directories("${CMAKE_SOURCE_DIR}/tests/cunit")

# Compiler-specific compiler options
if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "PGI")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -c99")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -c99")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "Intel")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")

#==============================================================================
#  PREPARE FOR TESTING
#==============================================================================

# Don't run these tests if we are using MPI SERIAL.
if (NOT PIO_USE_MPISERIAL)
  add_executable (test_intercomm2 EXCLUDE_FROM_ALL test_intercomm2.c test_common.c)
  target_link_libraries (test_intercomm2 pioc)
  add_dependencies (tests test_intercomm2)
  add_executable (test_async_simple EXCLUDE_FROM_ALL test_async_simple.c test_common.c)
  target_link_libraries (test_async_simple pioc)
  add_dependencies (tests test_async_simple)
  add_executable (test_async_3proc EXCLUDE_FROM_ALL test_async_3proc.c test_common.c)
  target_link_libraries (test_async_3proc pioc)
  add_dependencies (tests test_async_3proc)
  add_executable (test_async_4proc EXCLUDE_FROM_ALL test_async_4proc.c test_common.c)
  target_link_libraries (test_async_4proc pioc)
  add_dependencies (tests test_async_4proc)
  add_executable (test_iosystem2_simple EXCLUDE_FROM_ALL test_iosystem2_simple.c test_common.c)
  target_link_libraries (test_iosystem2_simple pioc)
  add_dependencies (tests test_iosystem2_simple)
  add_executable (test_iosystem2_simple2 EXCLUDE_FROM_ALL test_iosystem2_simple2.c test_common.c)
  target_link_libraries (test_iosystem2_simple2 pioc)
  add_dependencies (tests test_iosystem2_simple2)
  add_executable (test_iosystem2 EXCLUDE_FROM_ALL test_iosystem2.c test_common.c)
  target_link_libraries (test_iosystem2 pioc)
  add_dependencies (tests test_iosystem2)
  add_executable (test_iosystem3_simple EXCLUDE_FROM_ALL test_iosystem3_simple.c test_common.c)
  target_link_libraries (test_iosystem3_simple pioc)
  add_dependencies (tests test_iosystem3_simple)
  add_executable (test_iosystem3_simple2 EXCLUDE_FROM_ALL test_iosystem3_simple2.c test_common.c)
  target_link_libraries (test_iosystem3_simple2 pioc)
  add_dependencies (tests test_iosystem3_simple2)
  add_executable (test_iosystem3 EXCLUDE_FROM_ALL test_iosystem3.c test_common.c)
  target_link_libraries (test_iosystem3 pioc)
  add_dependencies (tests test_iosystem3)
  add_executable (test_pioc EXCLUDE_FROM_ALL test_pioc.c test_common.c)
  target_link_libraries (test_pioc pioc)
endif ()
add_executable (test_spmd EXCLUDE_FROM_ALL test_spmd.c test_common.c)
target_link_libraries (test_spmd pioc)
add_dependencies (tests test_spmd)
add_executable (test_darray EXCLUDE_FROM_ALL test_darray.c test_common.c)
target_link_libraries (test_darray pioc)

add_dependencies (tests test_darray)
add_dependencies (tests test_pioc)

# Test Timeout in seconds.
set (DEFAULT_TEST_TIMEOUT 60)

# All tests need a certain number of tasks, but they should be able to
# run successfully with more than they need. Test this by providing an
# extra processor for each C test.
set (AT_LEAST_TWO_TASKS 3)
set (AT_LEAST_THREE_TASKS 4)
set (AT_LEAST_FOUR_TASKS 5)

if (PIO_USE_MPISERIAL)
  add_test(NAME test_pioc
    COMMAND test_pioc)
else ()
  add_mpi_test(test_darray
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/test_darray
    NUMPROCS ${AT_LEAST_FOUR_TASKS}
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(test_spmd
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/test_spmd
    NUMPROCS ${AT_LEAST_FOUR_TASKS}
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(test_intercomm2
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/test_intercomm2
    NUMPROCS ${AT_LEAST_FOUR_TASKS}
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(test_async_simple
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/test_async_simple
    NUMPROCS ${AT_LEAST_TWO_TASKS}
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(test_async_3proc
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/test_async_3proc
    NUMPROCS ${AT_LEAST_THREE_TASKS}
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(test_async_4proc
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/test_async_4proc
    NUMPROCS ${AT_LEAST_FOUR_TASKS}
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(test_iosystem2_simple
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/test_iosystem2_simple
    NUMPROCS ${AT_LEAST_TWO_TASKS}
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(test_iosystem2_simple2
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/test_iosystem2_simple2
    NUMPROCS ${AT_LEAST_FOUR_TASKS}
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(test_iosystem2
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/test_iosystem2
    NUMPROCS ${AT_LEAST_FOUR_TASKS}
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(test_iosystem3_simple
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/test_iosystem3_simple
    NUMPROCS ${AT_LEAST_FOUR_TASKS}
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(test_iosystem3_simple2
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/test_iosystem3_simple2
    NUMPROCS ${AT_LEAST_FOUR_TASKS}
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(test_iosystem3
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/test_iosystem3
    NUMPROCS ${AT_LEAST_FOUR_TASKS}
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
  add_mpi_test(test_pioc
    EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/test_pioc
    NUMPROCS ${AT_LEAST_FOUR_TASKS}
    TIMEOUT ${DEFAULT_TEST_TIMEOUT})
endif ()
