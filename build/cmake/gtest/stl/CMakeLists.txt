
set(gs_test_stl_path ${gs_project_gtest_path}/stl)

macro(mc_create_test1 sCxxName)
    aux_source_directory(${gs_test_ccxx_path}/${sCxxName} gsl_${sCxxName}_DIR_SRCS)

    add_executable(gtest_${sCxxName} ${gsl_${sCxxName}_DIR_SRCS})

    add_dependencies(gtest_${sCxxName} gtest gtest_main)

    mc_target_link_libraries(gtest_${sCxxName} gtest gtest_main ccxx_main_static)

    set_target_properties(gtest_${sCxxName} PROPERTIES FOLDER gtest)

    add_test(NAME gtest_${sCxxName} COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gtest_${sCxxName})
endmacro()

add_subdirectory(string)

add_subdirectory(vector)
