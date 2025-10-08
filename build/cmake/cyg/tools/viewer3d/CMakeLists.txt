
# app
set(src_path ${gs_project_cyg_tools_path}/viewer3d)

include_directories(${src_path})

file(GLOB src_header_filepaths
        "${src_path}/*.h"
)

file(GLOB src_cpp_filepaths
        "${src_path}/*.cpp"
)

file(GLOB src_ui_filepaths
        "${src_path}/*.ui"
)

file(GLOB src_qss_filepaths
        "${src_path}/*.qss"
)

add_executable(cx_mct_viewer3d
        ${src_header_filepaths}
        ${src_cpp_filepaths}
        ${src_ui_filepaths}
)

#message("--- --- --- VTK_LIBRARIES: ${VTK_LIBRARIES}")

target_link_libraries(cx_mct_viewer3d PRIVATE
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
    target_link_libraries(cx_mct_viewer3d PRIVATE Qt6::GuiPrivate) #needed for <qpa/qplatformnativeinterface.h>
endif()

if (UNIX AND NOT APPLE)
    if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
        find_package(X11 REQUIRED)
        target_link_libraries(cx_mct_viewer3d PUBLIC X11::xcb)
    else()
        target_link_libraries(cx_mct_viewer3d PUBLIC xcb)
    endif()
endif()

vtk_module_autoinit(
        TARGETS cx_mct_viewer3d
        MODULES ${VTK_LIBRARIES}
)

foreach(qss_file ${src_qss_filepaths})
    add_custom_command(
            TARGET cx_mct_viewer3d
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${qss_file}"
            "$<TARGET_FILE_DIR:cx_mct_viewer3d>/qss"
            COMMENT "Copying QSS file: ${qss_file}"
    )
endforeach()