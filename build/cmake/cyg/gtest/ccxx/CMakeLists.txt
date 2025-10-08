
add_definitions(-D"GM_BUILD_CCXX_CORE_CODE")

macro(mc_create_gtest1 sCxxName)
    aux_source_directory(${gs_project_cyg_ccxx_path}/${sCxxName} gsl_${sCxxName}_DIR_SRCS)

    add_executable(gtest_cyg_${sCxxName}
            ${gsl_${sCxxName}_DIR_SRCS}
    )

    # 链接 gtest_main（自带 main()）
    target_link_libraries(gtest_cyg_${sCxxName} PRIVATE
            ccxx
            GTest::gtest
            GTest::gtest_main
            wsock32 ws2_32 psapi
    )

#    # 自动发现 TEST()
#    include(GoogleTest)
#    gtest_discover_tests(gtest_cyg_${sCxxName} DISCOVERY_TIMEOUT 60)
#
#    add_test(NAME gtest_cyg_${sCxxName} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gtest_cyg_${sCxxName})
#
#    set_target_properties(gtest_cyg_${sCxxName} PROPERTIES FOLDER gtest)
endmacro()

mc_create_gtest1(cxstring)

mc_create_gtest1(cxfile)
