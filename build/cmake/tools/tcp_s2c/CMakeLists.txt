cmake_minimum_required(VERSION 3.12...3.21 FATAL_ERROR)

project(tcp_s2c LANGUAGES CXX)

# uv
include_directories(${gs_project_3rd_path}/libuv/include)

# ccxx
#add_definitions(-D"GM_BUILD_CCXX_CORE_CODE")
include_directories(${gs_project_ccxx_path})

# projects
include_directories(${gs_project_tk_path})

# qpp
set(gs_tcp_s2c_src_path ${gs_project_tools_path}/tcp_s2c)

add_executable(tcp_s2c
#        ${gs_tcp_s2c_src_path}/main_tcp_server.cpp
        ${gs_tcp_s2c_src_path}/main_s2c.cpp
        ${gs_tcp_s2c_src_path}/uv_tcp_server.cpp
        ${gs_tcp_s2c_src_path}/uv_tcp_client.cpp
)

#mc_target_link_libraries(tcp_s2c ccxx_main_static uv_a)

target_link_libraries(tcp_s2c ccxx_main_static uv_a wsock32 ws2_32 psapi)
