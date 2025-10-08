
# qt

find_package(Qt6 COMPONENTS Core Gui GuiPrivate Widgets REQUIRED)

if (UNIX AND NOT APPLE AND ((QT_VERSION_MAJOR STREQUAL "6" AND QT_VERSION_MINOR GREATER 8) OR QT_VERSION_MAJOR STREQUAL "5"))
    include_directories(${Qt${QT_VERSION_MAJOR}Gui_PRIVATE_INCLUDE_DIRS})
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

if (UNIX AND NOT APPLE)
    set(src_cpps linux/FloatingWidgetTitleBar.cpp ${src_cpps})
    set(src_headers linux/FloatingWidgetTitleBar.h ${src_headers})
endif()

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# docking
include(../../common/docking.cmake)

# app

set(src_headers MainWindow.h droppableitem.h)
set(src_cpps main.cpp MainWindow.cpp droppableitem.cpp)
set(src_uis MainWindow.ui)

set(src_path ${gs_project_demo_path}/docking2)

mc_merge_file_path(${src_path} "${src_headers}" src_header_filepaths)
mc_merge_file_path(${src_path} "${src_cpps}" src_cpp_filepaths)
mc_merge_file_path(${src_path} "${src_uis}" src_ui_filepaths)

add_definitions(-D"ADS_STATIC")

add_executable(demo_docking2
        # docking
        ${src_docking_header_filepaths}
        ${src_docking_cpp_filepaths}
        # docking2
        ${src_header_filepaths}
        ${src_cpp_filepaths}
        ${src_ui_filepaths}
)

target_link_libraries(demo_docking2 PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
)

if(QT_VERSION_MAJOR STREQUAL "6")
    target_link_libraries(demo_docking2 PRIVATE Qt6::GuiPrivate) #needed for <qpa/qplatformnativeinterface.h>
endif()

if (UNIX AND NOT APPLE)
    if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
        find_package(X11 REQUIRED)
        target_link_libraries(demo_docking2 PUBLIC X11::xcb)
    else()
        target_link_libraries(demo_docking2 PUBLIC xcb)
    endif()
endif()

#set_target_properties(demo_docking2 PROPERTIES
#        AUTOMOC ON
#        AUTORCC ON
#        CXX_EXTENSIONS OFF
#        VERSION ${VERSION_SHORT}
#        EXPORT_NAME ${library_name}
#        DEBUG_POSTFIX "d"
#        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/lib"
#        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/lib"
#        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${ads_PlatformDir}/bin"
#)

#include(CMakePackageConfigHelpers)
#write_basic_package_version_file(
#        "${library_name}ConfigVersion.cmake"
#        VERSION ${VERSION_SHORT}
#        COMPATIBILITY SameMajorVersion
#)

#target_include_directories(${library_name} PUBLIC
#        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
#)
