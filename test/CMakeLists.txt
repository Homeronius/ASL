# Tests for the main library.
add_executable(hdbscan_test EXCLUDE_FROM_ALL
    hdbscan_test.cpp
)
target_link_libraries(hdbscan_test hdbscan_lib)

# Same tests for vectorized version
add_executable(hdbscan_test_vec EXCLUDE_FROM_ALL
    hdbscan_test.cpp
)
target_link_libraries(hdbscan_test_vec hdbscan_lib_vec)

# GTest libs
if(DEFINED ENV{GTEST_ROOT})
  target_link_libraries(hdbscan_test GTest::GTest GTest::Main)
  target_link_libraries(hdbscan_test_vec GTest::GTest GTest::Main)
else()
  target_link_libraries(hdbscan_test gtest_main)
  target_link_libraries(hdbscan_test_vec gtest_main)
endif()

include(GoogleTest)
gtest_discover_tests(hdbscan_test)
gtest_discover_tests(hdbscan_test_vec)

# Run tests when invoking `ninja check`
# Not very elegant running vectorized and non-vectorized tests at the same time
# but quickest version for now.
add_dependencies(check hdbscan_test hdbscan_test_vec)
