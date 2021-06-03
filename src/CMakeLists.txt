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
  quickselect.cpp
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
add_library(hdbscan_lib_distvec
  distance_vectorized.cpp
  union_find.cpp
  clustering.cpp
  utils.cpp
  prim.cpp
  hdbscan.cpp
  quickselect.cpp
)
target_include_directories(hdbscan_lib_distvec PUBLIC ${HDBSCAN_INCLUDE_DIR})
set_target_properties(hdbscan_lib_distvec PROPERTIES COMPILE_FLAGS "-save-temps")
add_executable(hdbscan_benchmark_distvec EXCLUDE_FROM_ALL
  benchmark_main.cpp
)
target_link_libraries(hdbscan_benchmark_distvec PRIVATE
  hdbscan_lib_distvec
  hdbscan_benchmark_lib
)
add_executable(hdbscan_distvec main.cpp)
target_link_libraries(hdbscan_distvec PRIVATE hdbscan_lib_distvec)

###########################################
#### VECTORIZED DISTANCE + QUICKSELECT ####
###########################################
add_library(hdbscan_lib_distvec_quickvec
  distance_vectorized.cpp
  union_find.cpp
  clustering.cpp
  utils.cpp
  prim.cpp
  hdbscan.cpp
  quickselect_vectorized.cpp
)
target_include_directories(hdbscan_lib_distvec_quickvec PUBLIC ${HDBSCAN_INCLUDE_DIR})
set_target_properties(hdbscan_lib_distvec_quickvec PROPERTIES COMPILE_FLAGS "-save-temps")
add_executable(hdbscan_benchmark_distvec_quickvec EXCLUDE_FROM_ALL
  benchmark_main.cpp
)
target_link_libraries(hdbscan_benchmark_distvec_quickvec PRIVATE
  hdbscan_lib_distvec_quickvec
  hdbscan_benchmark_lib
)
add_executable(hdbscan_distvec_quickvec main.cpp)
target_link_libraries(hdbscan_distvec_quickvec PRIVATE hdbscan_lib_distvec_quickvec)

#############################################
#### VECT. DISTANCE + QUICKSELECT + PRIM ####
#############################################
add_library(hdbscan_lib_distvec_quickvec_primvec
  distance_vectorized.cpp
  union_find.cpp
  clustering.cpp
  utils.cpp
  prim_vectorized.cpp
  hdbscan.cpp
  quickselect_vectorized.cpp
)
target_include_directories(hdbscan_lib_distvec_quickvec_primvec PUBLIC ${HDBSCAN_INCLUDE_DIR})
set_target_properties(hdbscan_lib_distvec_quickvec_primvec PROPERTIES COMPILE_FLAGS "-save-temps")
add_executable(hdbscan_benchmark_distvec_quickvec_primvec EXCLUDE_FROM_ALL
  benchmark_main.cpp
)
target_link_libraries(hdbscan_benchmark_distvec_quickvec_primvec PRIVATE
  hdbscan_lib_distvec_quickvec_primvec
  hdbscan_benchmark_lib
)
add_executable(hdbscan_distvec_quickvec_primvec main.cpp)
target_link_libraries(hdbscan_distvec_quickvec_primvec PRIVATE hdbscan_lib_distvec_quickvec_primvec)

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
  quickselect.cpp
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

####################################
#### TRIANGULAR DISTANCE MATRIX ####
####################################

add_library(hdbscan_basic_lib_triang
  distance.cpp
  union_find.cpp
  clustering.cpp
  utils.cpp
  prim.cpp
  hdbscan_basic_triangular.cpp
  quickselect.cpp
)

target_include_directories(hdbscan_basic_lib_triang PUBLIC ${HDBSCAN_INCLUDE_DIR})
set_target_properties(hdbscan_basic_lib_triang PROPERTIES COMPILE_FLAGS "-save-temps")
add_executable(hdbscan_basic_benchmark_triang EXCLUDE_FROM_ALL
  benchmark_main.cpp
)
target_link_libraries(hdbscan_basic_benchmark_triang PRIVATE
  hdbscan_basic_lib_triang
  hdbscan_basic_benchmark_lib
)
add_executable(hdbscan_basic_triang main.cpp)
target_link_libraries(hdbscan_basic_triang PRIVATE hdbscan_basic_lib_triang)

#################################
#### BLOCKED DISTANCE MATRIX ####
#################################

add_library(hdbscan_basic_lib_blocked
  distance.cpp
  union_find.cpp
  clustering.cpp
  utils.cpp
  prim.cpp
  hdbscan_basic_blocked.cpp
  quickselect.cpp
)

target_include_directories(hdbscan_basic_lib_blocked PUBLIC ${HDBSCAN_INCLUDE_DIR})
set_target_properties(hdbscan_basic_lib_blocked PROPERTIES COMPILE_FLAGS "-save-temps")
add_executable(hdbscan_basic_benchmark_blocked EXCLUDE_FROM_ALL
  benchmark_main.cpp
)
target_link_libraries(hdbscan_basic_benchmark_blocked PRIVATE
  hdbscan_basic_lib_blocked
  hdbscan_basic_benchmark_lib
)
add_executable(hdbscan_basic_blocked main.cpp)
target_link_libraries(hdbscan_basic_blocked PRIVATE hdbscan_basic_lib_blocked)

#############################
#### VECTORIZED DISTANCE ####
#############################
add_library(hdbscan_basic_lib_distvec
  distance_vectorized.cpp
  union_find.cpp
  clustering.cpp
  utils.cpp
  prim.cpp
  hdbscan_basic.cpp
  quickselect.cpp
)
target_include_directories(hdbscan_basic_lib_distvec PUBLIC ${HDBSCAN_INCLUDE_DIR})
set_target_properties(hdbscan_basic_lib_distvec PROPERTIES COMPILE_FLAGS "-save-temps")
add_executable(hdbscan_basic_benchmark_distvec EXCLUDE_FROM_ALL
  benchmark_main.cpp
)
target_link_libraries(hdbscan_basic_benchmark_distvec PRIVATE
  hdbscan_basic_lib_distvec
  hdbscan_basic_benchmark_lib
)
add_executable(hdbscan_basic_distvec main.cpp)
target_link_libraries(hdbscan_basic_distvec PRIVATE hdbscan_basic_lib_distvec)

###########################################
#### VECTORIZED DISTANCE + QUICKSELECT ####
###########################################
add_library(hdbscan_basic_lib_distvec_quickvec
  distance_vectorized.cpp
  union_find.cpp
  clustering.cpp
  utils.cpp
  prim.cpp
  hdbscan_basic.cpp
  quickselect_vectorized.cpp
)
target_include_directories(hdbscan_basic_lib_distvec_quickvec PUBLIC ${HDBSCAN_INCLUDE_DIR})
set_target_properties(hdbscan_basic_lib_distvec_quickvec PROPERTIES COMPILE_FLAGS "-save-temps")
add_executable(hdbscan_basic_benchmark_distvec_quickvec EXCLUDE_FROM_ALL
  benchmark_main.cpp
)
target_link_libraries(hdbscan_basic_benchmark_distvec_quickvec PRIVATE
  hdbscan_basic_lib_distvec_quickvec
  hdbscan_basic_benchmark_lib
)
add_executable(hdbscan_basic_distvec_quickvec main.cpp)
target_link_libraries(hdbscan_basic_distvec_quickvec PRIVATE hdbscan_basic_lib_distvec_quickvec)


#############################################
#### VECT. DISTANCE + QUICKSELECT + PRIM ####
#############################################
add_library(hdbscan_basic_lib_distvec_quickvec_primvec
  distance_vectorized.cpp
  union_find.cpp
  clustering.cpp
  utils.cpp
  prim_vectorized.cpp
  hdbscan_basic.cpp
  distance_vectorized.cpp
  quickselect_vectorized.cpp
)
target_include_directories(hdbscan_basic_lib_distvec_quickvec_primvec PUBLIC ${HDBSCAN_INCLUDE_DIR})
set_target_properties(hdbscan_basic_lib_distvec_quickvec_primvec PROPERTIES COMPILE_FLAGS "-save-temps")
add_executable(hdbscan_basic_benchmark_distvec_quickvec_primvec EXCLUDE_FROM_ALL
  benchmark_main.cpp
)
target_link_libraries(hdbscan_basic_benchmark_distvec_quickvec_primvec PRIVATE
  hdbscan_basic_lib_distvec_quickvec_primvec
  hdbscan_basic_benchmark_lib
)
add_executable(hdbscan_basic_distvec_quickvec_primvec main.cpp)
target_link_libraries(hdbscan_basic_distvec_quickvec_primvec PRIVATE hdbscan_basic_lib_distvec_quickvec_primvec)

#
#############################
####### END PRIM BASIC ######
#############################
