
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

# app

set(src_path ${gs_project_demo_path}/uiframe1)

file(GLOB src_header_filepaths
        "${src_path}/*.h"
        "${src_path}/panels/*.h"
)

file(GLOB src_cpp_filepaths
        "${src_path}/*.cpp"
        "${src_path}/panels/*.cpp"
        "${src_path}/framelesswindow/*.cpp"
)

file(GLOB src_qrc_filepaths "${src_path}/*.qrc")

file(GLOB src_ui_filepaths
        "${src_path}/*.ui"
        "${src_path}/panels/*.ui"
        "${src_path}/framelesswindow/*.ui"
)

#set(src_headers MainWindow.h droppableitem.h)
#set(src_cpps main.cpp MainWindow.cpp droppableitem.cpp)
#set(src_uis MainWindow.ui)
#
#mc_merge_file_path(${src_path} "${src_headers}" src_header_filepaths)
#mc_merge_file_path(${src_path} "${src_cpps}" src_cpp_filepaths)
#mc_merge_file_path(${src_path} "${src_uis}" src_ui_filepaths)

add_executable(demo_uiframe1
        # uiframe1
        ${src_header_filepaths}
        ${src_cpp_filepaths}
        ${src_ui_filepaths}
        ${src_qrc_filepaths}
)

target_link_libraries(demo_uiframe1 PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets

)

if(QT_VERSION_MAJOR STREQUAL "6")
    target_link_libraries(demo_uiframe1 PRIVATE Qt6::GuiPrivate) #needed for <qpa/qplatformnativeinterface.h>
endif()

if (UNIX AND NOT APPLE)
    if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
        find_package(X11 REQUIRED)
        target_link_libraries(demo_uiframe1 PUBLIC X11::xcb)
    else()
        target_link_libraries(demo_uiframe1 PUBLIC xcb)
    endif()
endif()
