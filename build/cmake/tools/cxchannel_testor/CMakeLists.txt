cmake_minimum_required(VERSION 3.12...3.21 FATAL_ERROR)

project(channel_testor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# ccxx
include_directories(${gs_project_ccxx_path})
add_definitions(-D"GM_BUILD_CCXX_CORE_LIB")

# qt
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# qpp
set(gs_channel_testor_src_path ${gs_project_tools_path}/cxchannel_testor)

add_executable(channel_testor
        ${gs_channel_testor_src_path}/main.cpp
        ${gs_channel_testor_src_path}/test_channel_win.cpp
        ${gs_channel_testor_src_path}/test_channel_win.h
        ${gs_channel_testor_src_path}/test_channel_win.ui
        ${gs_ccxx_qt_paths}
)

target_link_libraries(channel_testor PRIVATE
        Qt6::Widgets
        wsock32
        ws2_32
        psapi
)
