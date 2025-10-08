
if (MSVC)
    add_definitions(-D"_CRT_SECURE_NO_WARNINGS")
    add_definitions( "/wd4819" )
    add_compile_options(/EHsc)
endif ()

set(gs_test_stl_path ${gs_project_gtest_path}/stl)

add_subdirectory(stl)

add_subdirectory(ccxx)
