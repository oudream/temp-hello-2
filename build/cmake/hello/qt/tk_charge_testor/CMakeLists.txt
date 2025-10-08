cmake_minimum_required(VERSION 3.12...3.21 FATAL_ERROR)

project(tk_charge_testor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# ccxx
#add_definitions(-D"GM_BUILD_CCXX_CORE_CODE")
include_directories(${gs_project_ccxx_path})

# qt
find_package(Qt6 COMPONENTS Widgets REQUIRED)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# qpp
set(gs_charge_testor_src_path ${gs_project_tk_path}/tk_charge_testor)

add_executable(tk_charge_testor
        ${gs_charge_testor_src_path}/main.cpp
        ${gs_charge_testor_src_path}/charge_testor_win.cpp
        ${gs_charge_testor_src_path}/charge_testor_win.h
        ${gs_charge_testor_src_path}/charging_agreement.h
        ${gs_charge_testor_src_path}/charging_agreement.cpp
        ${gs_charge_testor_src_path}/git_port_name.h
        ${gs_charge_testor_src_path}/git_port_name.cpp
        ${gs_charge_testor_src_path}/dlt645/dlt645.h
        ${gs_charge_testor_src_path}/dlt645/dlt645.cpp
        ${gs_charge_testor_src_path}/dlt645/dlt645-1997.h
        ${gs_charge_testor_src_path}/dlt645/dlt645-1997.cpp
        ${gs_charge_testor_src_path}/dlt645/dlt645-2007.h
        ${gs_charge_testor_src_path}/dlt645/dlt645-2007.cpp
        ${gs_charge_testor_src_path}/dlt645/dlt645-private.h
        ${gs_charge_testor_src_path}/adapter.h
        ${gs_charge_testor_src_path}/charge_testor_win.ui
)

target_link_libraries(tk_charge_testor PRIVATE Qt6::Widgets ccxx_qt_static wsock32 ws2_32 psapi)
