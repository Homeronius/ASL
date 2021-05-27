option(
  OUTPUT_COMPILER_INFO
  "Output compiler information when launching the main executable." ON
)

configure_file (
  "${HDBSCAN_INCLUDE_DIR}/config.h.in"
  "${HDBSCAN_INCLUDE_DIR}/config.h"
)

######################################################
################### PRIM ADVANCED ####################
######################################################
add_library(hdbscan_lib
  distance.cpp
  union_find.cpp
  clustering.cpp
  utils.cpp
  prim.cpp
  hdbscan.cpp
)
target_include_directories(hdbscan_lib PUBLIC ${HDBSCAN_INCLUDE_DIR})
set_target_properties(hdbscan_lib PROPERTIES COMPILE_FLAGS "-save-temps")
add_executable(hdbscan main.cpp)
target_link_libraries(hdbscan PRIVATE hdbscan_lib)

################### 
#### BENCHMARK ####
###################
add_library(hdbscan_benchmark_lib EXCLUDE_FROM_ALL
  benchmark_util.cpp
)
target_include_directories(hdbscan_benchmark_lib PUBLIC ${HDBSCAN_INCLUDE_DIR})
add_executable(hdbscan_benchmark EXCLUDE_FROM_ALL
  benchmark_main.cpp
)
target_link_libraries(hdbscan_benchmark PRIVATE
  hdbscan_lib
  hdbscan_benchmark_lib
)
#############################
#### VECTORIZED DISTANCE ####
#############################
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
add_executable(hdbscan_benchmark_vec EXCLUDE_FROM_ALL
  benchmark_main.cpp
)
target_link_libraries(hdbscan_benchmark_vec PRIVATE
  hdbscan_lib_vec
  hdbscan_benchmark_lib
)
add_executable(hdbscan_vec main.cpp)
target_link_libraries(hdbscan_vec PRIVATE hdbscan_lib_vec)

####################################
#### VECTORIZED DISTANCE + PRIM ####
####################################
add_library(hdbscan_lib_vec_primvec
  distance_vectorized.cpp
  union_find.cpp
  clustering.cpp
  utils.cpp
  prim_vectorized.cpp
  hdbscan.cpp
)
target_include_directories(hdbscan_lib_vec_primvec PUBLIC ${HDBSCAN_INCLUDE_DIR})
set_target_properties(hdbscan_lib_vec_primvec PROPERTIES COMPILE_FLAGS "-save-temps")
add_executable(hdbscan_benchmark_vec_primvec EXCLUDE_FROM_ALL
  benchmark_main.cpp
)
target_link_libraries(hdbscan_benchmark_vec_primvec PRIVATE
  hdbscan_lib_vec_primvec
  hdbscan_benchmark_lib
)
add_executable(hdbscan_vec_primvec main.cpp)
target_link_libraries(hdbscan_vec_primvec PRIVATE hdbscan_lib_vec_primvec)

######################################################
################ END PRIM ADVANCED ###################
######################################################


######################################################
################### PRIM BASIC #######################
######################################################
# the version with precomputation of distances
add_library(hdbscan_basic_lib
  distance.cpp
  union_find.cpp
  clustering.cpp
  utils.cpp
  prim.cpp
  hdbscan_basic.cpp
)
target_include_directories(hdbscan_basic_lib PUBLIC ${HDBSCAN_INCLUDE_DIR})
set_target_properties(hdbscan_basic_lib PROPERTIES COMPILE_FLAGS "-save-temps")
add_executable(hdbscan_basic main.cpp)
target_link_libraries(hdbscan_basic PRIVATE hdbscan_basic_lib)

################### 
#### BENCHMARK ####
###################
add_library(hdbscan_basic_benchmark_lib EXCLUDE_FROM_ALL
  benchmark_util.cpp
)
target_include_directories(hdbscan_basic_benchmark_lib PUBLIC ${HDBSCAN_INCLUDE_DIR})
add_executable(hdbscan_basic_benchmark EXCLUDE_FROM_ALL
  benchmark_main.cpp
)
target_link_libraries(hdbscan_basic_benchmark PRIVATE
  hdbscan_basic_lib
  hdbscan_basic_benchmark_lib
)


#############################
#### VECTORIZED DISTANCE ####
#############################
add_library(hdbscan_basic_lib_vec
  distance_vectorized.cpp
  union_find.cpp
  clustering.cpp
  utils.cpp
  prim.cpp
  hdbscan_basic.cpp
)
target_include_directories(hdbscan_basic_lib_vec PUBLIC ${HDBSCAN_INCLUDE_DIR})
set_target_properties(hdbscan_basic_lib_vec PROPERTIES COMPILE_FLAGS "-save-temps")
add_executable(hdbscan_basic_benchmark_vec EXCLUDE_FROM_ALL
  benchmark_main.cpp
)
target_link_libraries(hdbscan_basic_benchmark_vec PRIVATE
  hdbscan_basic_lib_vec
  hdbscan_basic_benchmark_lib
)
add_executable(hdbscan_basic_vec main.cpp)
target_link_libraries(hdbscan_basic_vec PRIVATE hdbscan_basic_lib_vec)

####################################
#### VECTORIZED DISTANCE + PRIM ####
####################################
add_library(hdbscan_basic_lib_vec_primvec
  distance_vectorized.cpp
  union_find.cpp
  clustering.cpp
  utils.cpp
  prim_vectorized.cpp
  hdbscan_basic.cpp
)
target_include_directories(hdbscan_basic_lib_vec_primvec PUBLIC ${HDBSCAN_INCLUDE_DIR})
set_target_properties(hdbscan_basic_lib_vec_primvec PROPERTIES COMPILE_FLAGS "-save-temps")
add_executable(hdbscan_basic_benchmark_vec_primvec EXCLUDE_FROM_ALL
  benchmark_main.cpp
)
target_link_libraries(hdbscan_basic_benchmark_vec_primvec PRIVATE
  hdbscan_basic_lib_vec_primvec
  hdbscan_basic_benchmark_lib
)
add_executable(hdbscan_basic_vec_primvec main.cpp)
target_link_libraries(hdbscan_basic_vec_primvec PRIVATE hdbscan_basic_lib_vec_primvec)

#
#############################
####### END PRIM BASIC ######
#############################
