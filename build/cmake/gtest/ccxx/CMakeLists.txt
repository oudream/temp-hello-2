
add_definitions(-D"GM_BUILD_CCXX_CORE_CODE")

macro(mc_create_gtest1 sCxxName)
    add_executable(gtest_${sCxxName}
            ${gs_project_ccxx_path}/${sCxxName}.cpp
    )

    # 链接 gtest_main（自带 main()）
    target_link_libraries(gtest_${sCxxName} PRIVATE
            ccxx
            GTest::gtest
            GTest::gtest_main
            wsock32 ws2_32 psapi
    )

#    # 自动发现 TEST()
#    include(GoogleTest)
#    gtest_discover_tests(gtest_${sCxxName} DISCOVERY_TIMEOUT 60)
#
#    add_test(NAME gtest_${sCxxName} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gtest_${sCxxName})
#
#    set_target_properties(gtest_${sCxxName} PROPERTIES FOLDER gtest)
endmacro()

mc_create_gtest1(cxstring_test)

mc_create_gtest1(cxfilesystem_test)
