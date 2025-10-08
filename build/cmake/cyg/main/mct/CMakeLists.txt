
# app

set(src_path ${gs_project_cyg_main_path}/mct)

include_directories(${src_path})

file(GLOB src_header_filepaths
        "${src_path}/*.h"
        "${src_path}/base/*.h"
        "${src_path}/mc/*.h"
        "${src_path}/ui/*.h"
        "${src_path}/ui/pages/*.h"
        "${src_path}/ui/panels/*.h"
        "${src_path}/ui/views/*.h"
        "${src_path}/ui/adapters/*.h"
        "${src_path}/ui/dialogs/*.h"
        "${src_path}/test/vtk/*.h"
)

file(GLOB src_cpp_filepaths
        "${src_path}/*.cpp"
        "${src_path}/base/*.cpp"
        "${src_path}/mc/*.cpp"
        "${src_path}/ui/*.cpp"
        "${src_path}/ui/pages/*.cpp"
        "${src_path}/ui/panels/*.cpp"
        "${src_path}/ui/views/*.cpp"
        "${src_path}/ui/adapters/*.cpp"
        "${src_path}/ui/dialogs/*.cpp"
        "${src_path}/test/vtk/*.cpp"
)

file(GLOB src_ui_filepaths
        "${src_path}/*.ui"
        "${src_path}/ui/*.ui"
        "${src_path}/ui/pages/.ui"
        "${src_path}/ui/panels/.ui"
        "${src_path}/ui/views/.ui"
        "${src_path}/ui/adapters/.ui"
        "${src_path}/ui/dialogs/.ui"
)

file(GLOB src_qss_filepaths
        "${src_path}/qss/*.qss"
        "${src_path}/*.qss"
)

add_executable(cx_mct_studio
        # helpers
        ${src_helpers_simple_header_filepaths}
        ${src_helpers_simple_cpp_filepaths}
        # src
        ${src_header_filepaths}
        ${src_cpp_filepaths}
        ${src_ui_filepaths}
)

message("--- --- --- VTK_LIBRARIES: ${VTK_LIBRARIES}")

target_link_libraries(cx_mct_studio PRIVATE
        ccxx
        vtkITK
        vtkSegmentationCore
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        ${VTK_LIBRARIES}
#        ${VTK_TARGETS}
        ${OpenCV_LIBS}
        ${ITK_LIBRARIES}
        nlohmann_json::nlohmann_json
        yaml-cpp::yaml-cpp
)

if(QT_VERSION_MAJOR STREQUAL "6")
    target_link_libraries(cx_mct_studio PRIVATE Qt6::GuiPrivate) #needed for <qpa/qplatformnativeinterface.h>
endif()

if (UNIX AND NOT APPLE)
    if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
        find_package(X11 REQUIRED)
        target_link_libraries(cx_mct_studio PUBLIC X11::xcb)
    else()
        target_link_libraries(cx_mct_studio PUBLIC xcb)
    endif()
endif()

vtk_module_autoinit(
        TARGETS cx_mct_studio
        MODULES ${VTK_LIBRARIES}
)

foreach(qss_file ${src_qss_filepaths})
    add_custom_command(
            TARGET cx_mct_studio
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${qss_file}"
            "$<TARGET_FILE_DIR:cx_mct_studio>/qss"
            COMMENT "Copying QSS file: ${qss_file}"
    )
endforeach()