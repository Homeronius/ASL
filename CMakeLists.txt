cmake_minimum_required(VERSION 3.17)
project(hdbscan)

set(CMAKE_VERBOSE_MAKEFILE on)

# In case we want to use some C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# if(OPT_LEVEL)
#     # Default compiler flags with custom optimization level
#     message("Running with custom optimization level: -${OPT_LEVEL}")
#     set(CMAKE_CXX_FLAGS "-${OPT_LEVEL} -g -march=native -W -Wall -pedantic") 
# else()
#     # Default compiler flags
#     set(CMAKE_CXX_FLAGS "-O3 -march=native -W -Wall -pedantic")
#     # set(CMAKE_CXX_FLAGS "-O3 -march=native -fomit-frame-pointer -fno-rtti -ffast-math -ffp-contract=fast -ffinite-math-only -fno-signed-zeros -fno-complete-member-pointers -ffunction-sections")
# endif()

# Useful for debugging vectorized code without dealing with all kinds of
# optimizations.
# set(CMAKE_CXX_FLAGS "-O0 -mavx -mavx2 -msse -mfma -mbmi -mbmi2")

set(HDBSCAN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(HEADER_FILES
    ${HDBSCAN_INCLUDE_DIR}/distance.h
    ${HDBSCAN_INCLUDE_DIR}/union_find.h
    ${HDBSCAN_INCLUDE_DIR}/clustering.h
    ${HDBSCAN_INCLUDE_DIR}/cluster.h
    ${HDBSCAN_INCLUDE_DIR}/utils.h
    ${HDBSCAN_INCLUDE_DIR}/hdbscan.h
    ${HDBSCAN_INCLUDE_DIR}/quickselect.h
    ${HDBSCAN_INCLUDE_DIR}/distance.h
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

IF(HDBSCAN_VERBOSE MATCHES "1")
  ADD_DEFINITIONS(-D HDBSCAN_VERBOSE)
ENDIF()

IF(HDBSCAN_INSTRUMENT MATCHES "1")
  ADD_DEFINITIONS(-D HDBSCAN_INSTRUMENT)
ENDIF()

IF(NOT DEFINED HDBSCAN_DATA_DIR)
  set(HDBSCAN_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)
ENDIF()

IF(BENCHMARK_AMD MATCHES "1")
  ADD_DEFINITIONS(-D BENCHMARK_AMD)
ENDIF()

IF(HDBSCAN_PRECOMPUTE_DIST_TRIANG MATCHES "1")
  ADD_DEFINITIONS(-D HDBSCAN_PRECOMPUTE_DIST_TRIANG)
ENDIF()

IF(HDBSCAN_PRECOMPUTE_DIST_BLOCKED MATCHES "1")
  ADD_DEFINITIONS(-D HDBSCAN_PRECOMPUTE_DIST_BLOCKED)
ENDIF()

IF(PACKLEFT_WLOOKUP MATCHES "1")
  ADD_DEFINITIONS(-D PACKLEFT_WLOOKUP)
ENDIF()

IF(HDBSCAN_QUICKSELECT MATCHES "1")
  ADD_DEFINITIONS(-D HDBSCAN_QUICKSELECT)
ENDIF()

add_custom_target(build_bench
  DEPENDS
    hdbscan_benchmark_lib
    hdbscan_benchmark
    hdbscan_basic_benchmark_lib
    hdbscan_basic_benchmark
    hdbscan_basic_benchmark_triang
    hdbscan_basic_benchmark_blocked
)
add_custom_target(build_bench_vec
  DEPENDS
    hdbscan_benchmark_lib
    hdbscan_benchmark_distvec
    hdbscan_benchmark_distvec_quickvec
    hdbscan_benchmark_distvec_quickvec_primvec
    hdbscan_basic_benchmark_distvec
    hdbscan_basic_benchmark_distvec_quickvec
    hdbscan_basic_benchmark_distvec_quickvec_primvec
)
add_custom_target(benchmark
  COMMAND
    # TODO: hardcoding input file is not nice
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/hdbscan_benchmark ${HDBSCAN_DATA_DIR}/perf_data_7.csv
  DEPENDS
    hdbscan_benchmark_lib
    hdbscan_benchmark
)
add_custom_target(benchmark_vec
  COMMAND
    # TODO: hardcoding input file is not nice
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/hdbscan_benchmark_vec ${HDBSCAN_DATA_DIR}/perf_data_7.csv
  DEPENDS
    hdbscan_benchmark_lib
    hdbscan_benchmark_vec
)

add_subdirectory(src)

IF(HDBSCAN_TEST MATCHES "1")
  enable_testing()
  add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

  if(DEFINED ENV{GTEST_ROOT})
    find_package(GTest REQUIRED)
  else()

    # The following block sets up a dependency on GTest as described at:
    # https://github.com/google/googletest/blob/master/googletest/README.md
    configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)

    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
      message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
      RESULT_VARIABLE result
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
      message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    # Prevent overriding the parent project's compiler/linker
    # settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                    ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                    EXCLUDE_FROM_ALL)

  endif()

  add_subdirectory(test)
ENDIF()
