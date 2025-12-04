
### depend path
#CMAKE_CURRENT_SOURCE_DIR (current file path)
set(gs_project_path ${CMAKE_CURRENT_SOURCE_DIR})

message('PROJECT-PATH: ' ${gs_project_path})
message(${CMAKE_C_COMPILER_ID})
message(${CMAKE_CXX_COMPILER_ID})
#message(${CMAKE_PREFIX_PATH})

set(gs_project_build_path ${gs_project_path}/build)
set(gs_project_include_path ${gs_project_path}/include)
set(gs_project_lib_path ${gs_project_path}/lib)
set(gs_project_common_path ${gs_project_path}/common)
set(gs_project_gtest_path ${gs_project_path}/gtest)
set(gs_project_benchmark_path ${gs_project_path}/benchmark)
set(gs_project_hello_path ${gs_project_path}/hello)
set(gs_project_tools_path ${gs_project_path}/tools)
set(gs_project_demo_path ${gs_project_path}/demo)
set(gs_project_cyg_path ${gs_project_path}/cyg)
set(gs_project_cmake_path ${gs_project_build_path}/cmake)

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(gs_project_deploy_path ${gs_project_path}/deploy/AppWin32)
else()
    set(gs_project_deploy_path ${gs_project_path}/deploy/AppWin64)
endif()

message("gs_project_deploy_path -" ${gs_project_deploy_path})

message("----------------------------------------------------------------------")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

### common
macro(mc_merge_file_path sDir sFileNames sOutFileNames)
    set(${sOutFileNames} "")
    #    message(${sFileNames})
    foreach (sFileName ${sFileNames})
        list(APPEND ${sOutFileNames} ${sDir}/${sFileName})
    endforeach ()
endmacro()

SET(EXECUTABLE_OUTPUT_PATH "${gs_project_deploy_path}")
SET(LIBRARY_OUTPUT_PATH "${gs_project_deploy_path}")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${gs_project_deploy_path}")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${gs_project_deploy_path}")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${gs_project_deploy_path}")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${gs_project_deploy_path}")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${gs_project_deploy_path}")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${gs_project_deploy_path}")
