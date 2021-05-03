add_library(hdbscan_lib
  distance.cpp
  union_find.cpp
  clustering.cpp
  utils.cpp
  prim.cpp
)
target_include_directories(hdbscan_lib PUBLIC ${HDBSCAN_INCLUDE_DIR})

add_library(hdbscan_benchmark_lib benchmark_util.cpp)
target_include_directories(hdbscan_benchmark_lib PUBLIC ${HDBSCAN_INCLUDE_DIR})

add_executable(hdbscan main.cpp)
target_link_libraries(hdbscan PRIVATE hdbscan_lib)

add_executable(hdbscan_benchmark benchmark_main.cpp)
target_link_libraries(hdbscan_benchmark PRIVATE
  hdbscan_lib
  hdbscan_benchmark_lib
)
# Save itermediate files to look at the assembly code
# set_target_properties(hdbscan_benchmark PROPERTIES COMPILE_FLAGS "-save-temps")
