cmake_minimum_required(VERSION 3.17)
project(hdbscan)

set(CMAKE_VERBOSE_MAKEFILE on)

# In case we want to use some C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_CXX_FLAGS "-O3 -march=native")

set(HDBSCAN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(HEADER_FILES
    ${HDBSCAN_INCLUDE_DIR}/distance.h
    ${HDBSCAN_INCLUDE_DIR}/union_find.h
    ${HDBSCAN_INCLUDE_DIR}/clustering.h
    ${HDBSCAN_INCLUDE_DIR}/cluster.h
    ${HDBSCAN_INCLUDE_DIR}/utils.h
    ${HDBSCAN_INCLUDE_DIR}/hdbscan.h
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

IF(HDBSCAN_VERBOSE MATCHES "1")
  ADD_DEFINITIONS(-D HDBSCAN_VERBOSE)
ENDIF()

enable_testing()
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
add_custom_target(benchmark COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/hdbscan_benchmark)

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


add_subdirectory(src)
add_subdirectory(test)
