
add_executable(gtest_stl_string
        ${gs_test_stl_path}/string/main.cpp
)

# 链接 gtest_main（自带 main()）
target_link_libraries(gtest_stl_string PRIVATE
        GTest::gtest
        GTest::gtest_main
)

# 自动发现 TEST()
include(GoogleTest)
gtest_discover_tests(gtest_stl_string
        DISCOVERY_TIMEOUT 60
)
