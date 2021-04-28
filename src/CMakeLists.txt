add_library(hdbscan_lib distance.cpp union_find.cpp clustering.cpp)
target_include_directories(hdbscan_lib PUBLIC ${HDBSCAN_INCLUDE_DIR})

add_executable(hdbscan main.cpp)
target_link_libraries(hdbscan PRIVATE hdbscan_lib)
