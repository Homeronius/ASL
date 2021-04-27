# Tests for the main library.
add_executable(hdbscan_test EXCLUDE_FROM_ALL 
    hdbscan_test.cpp
)
target_link_libraries(hdbscan_test hdbscan_lib)

# GTest libs
if(DEFINED ENV{GTEST_ROOT})
  target_link_libraries(hdbscan_test GTest::GTest GTest::Main)
else()
  target_link_libraries(hdbscan_test gtest_main)
endif()

include(GoogleTest)
gtest_discover_tests(hdbscan_test)

# Run tests when invoking `ninja check`
add_dependencies(check hdbscan_test)