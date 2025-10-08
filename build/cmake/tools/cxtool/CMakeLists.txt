cmake_minimum_required(VERSION 3.12...3.21 FATAL_ERROR)

project(cxtool LANGUAGES CXX)

# ccxx
#add_definitions(-D"GM_BUILD_CCXX_CORE_CODE")
include_directories(${gs_project_ccxx_path})
add_definitions(-D"GM_BUILD_CCXX_CORE_LIB")

# projects
include_directories(${gs_project_tk_path})

# qpp
set(gs_cxtool_src_path ${gs_project_tools_path}/cxtool)

add_executable(cxtool
        ${gs_cxtool_src_path}/main.cpp
        ${gs_ccxx_main_paths}
)

target_link_libraries(cxtool wsock32 ws2_32 psapi)
