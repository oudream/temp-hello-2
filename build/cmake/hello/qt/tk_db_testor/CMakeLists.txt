# gcc g++
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth=1800")

# compile warn
if (MSVC)
    add_definitions(-D"_CRT_SECURE_NO_WARNINGS")
    add_definitions( "/wd4819" )
endif ()

#
#
include_directories(${gs_project_include_path})

# ccxx
#add_definitions(-D"GM_BUILD_CCXX_CORE_CODE")
include_directories(${gs_project_ccxx_path})

# common
set(gsl_common_heads
        common/db_manager.h
        common/device_ddl.h
        common/device_manager.h
        common/point_manager.h
        )

set(gsl_common_sources
        common/db_manager.cpp
        common/device_manager.cpp
        common/point_manager.cpp
        )

mc_merge_file_path(${gs_project_tk_path} "${gsl_common_heads}" gsl_common_head_filepaths)
mc_merge_file_path(${gs_project_tk_path} "${gsl_common_sources}" gsl_common_source_filepaths)

source_group("common" FILES ${gsl_common_head_filepaths} ${gsl_common_source_filepaths})

include_directories(${gs_project_tk_path})

# tools
add_executable(tk_db_testor
        ${gs_project_tk_path}/tk_db_testor/main.cpp
        #
        ${gsl_common_head_filepaths} ${gsl_common_source_filepaths}
        #
        ${gs_project_ccxx_path}/ccxx/cxglobal.cpp
        ${gs_project_ccxx_path}/ccxx/cxstring.cpp
        ${gs_project_ccxx_path}/ccxx/cxtime.cpp
        ${gs_project_ccxx_path}/ccxx/cxsocket.cpp
        ${gs_project_ccxx_path}/ccxx/cxthread.cpp
        ${gs_project_ccxx_path}/ccxx/cxfile.cpp
        ${gs_project_ccxx_path}/ccxx/cxinterinfo.cpp
        ${gs_project_ccxx_path}/ccxx/cxarguments.cpp
        ${gs_project_ccxx_path}/ccxx/cxlog.cpp
)

mc_target_link_libraries(tk_db_testor ccxx_main_static)
