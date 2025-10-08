
add_executable(gtest_stl_vector
        ${gs_test_stl_path}/vector/main.cpp
)

# 链接 gtest_main（自带 main()）
target_link_libraries(gtest_stl_vector PRIVATE
        GTest::gtest
        GTest::gtest_main
)

# 自动发现 TEST()
include(GoogleTest)
gtest_discover_tests(gtest_stl_vector
        DISCOVERY_TIMEOUT 60
)
