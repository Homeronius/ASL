add_library(hdbscan_lib
  distance.cpp
  union_find.cpp
  clustering.cpp
  utils.cpp
  prim.cpp
  hdbscan.cpp
)

option(
    OUTPUT_COMPILER_INFO
    "Output compiler information when launching the main executable." ON
)

configure_file (
    "${HDBSCAN_INCLUDE_DIR}/config.h.in"
    "${HDBSCAN_INCLUDE_DIR}/config.h"
)

target_include_directories(hdbscan_lib PUBLIC ${HDBSCAN_INCLUDE_DIR})
set_target_properties(hdbscan_lib PROPERTIES COMPILE_FLAGS "-save-temps")

add_library(hdbscan_lib_vec
  distance_vectorized.cpp
  union_find.cpp
  clustering.cpp
  utils.cpp
  prim.cpp
  hdbscan.cpp
)
target_include_directories(hdbscan_lib_vec PUBLIC ${HDBSCAN_INCLUDE_DIR})
set_target_properties(hdbscan_lib_vec PROPERTIES COMPILE_FLAGS "-save-temps")

add_library(hdbscan_benchmark_lib EXCLUDE_FROM_ALL
  benchmark_util.cpp
)

target_include_directories(hdbscan_benchmark_lib PUBLIC ${HDBSCAN_INCLUDE_DIR})

add_executable(hdbscan main.cpp)
target_link_libraries(hdbscan PRIVATE hdbscan_lib)

add_executable(hdbscan_vec main.cpp)
target_link_libraries(hdbscan_vec PRIVATE hdbscan_lib_vec)

add_executable(hdbscan_benchmark EXCLUDE_FROM_ALL
  benchmark_main.cpp
)
target_link_libraries(hdbscan_benchmark PRIVATE
  hdbscan_lib
  hdbscan_benchmark_lib
)
add_executable(hdbscan_benchmark_vec EXCLUDE_FROM_ALL
  benchmark_main.cpp
)
target_link_libraries(hdbscan_benchmark_vec PRIVATE
  hdbscan_lib_vec
  hdbscan_benchmark_lib
)
