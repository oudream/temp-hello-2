
# app

set(src_path ${gs_project_cyg_main_path}/mct)

include_directories(${src_path})

file(GLOB src_header_filepaths
        "${src_path}/*.h"
        "${src_path}/base/*.h"
        "${src_path}/mc/*.h"
        "${src_path}/pages/*.h"
        "${src_path}/panels/*.h"
        "${src_path}/adapters/*.h"
        "${src_path}/dialogs/*.h"
        "${src_path}/test/vtk/*.h"
        "${src_path}/project/*.h"
        "${src_path}/acquisition/*.h"
        "${src_path}/reconstruction/*.h"
        "${src_path}/windowing/*.h"
        "${src_path}/inspection/*.h"
        "${src_path}/net/*.h"
)

file(GLOB src_cpp_filepaths
        "${src_path}/*.cpp"
        "${src_path}/base/*.cpp"
        "${src_path}/mc/*.cpp"
        "${src_path}/pages/*.cpp"
        "${src_path}/panels/*.cpp"
        "${src_path}/adapters/*.cpp"
        "${src_path}/dialogs/*.cpp"
        "${src_path}/test/vtk/*.cpp"
        "${src_path}/project/*.cpp"
        "${src_path}/acquisition/*.cpp"
        "${src_path}/reconstruction/*.cpp"
        "${src_path}/rendering/*.cpp"
        "${src_path}/windowing/*.cpp"
        "${src_path}/inspection/*.cpp"
        "${src_path}/net/*.cpp"
)

file(GLOB src_ui_filepaths
        "${src_path}/*.ui"
        "${src_path}/pages/.ui"
        "${src_path}/panels/.ui"
        "${src_path}/adapters/.ui"
        "${src_path}/dialogs/.ui"
        "${src_path}/project/*.ui"
        "${src_path}/acquisition/*.ui"
        "${src_path}/reconstruction/*.ui"
        "${src_path}/windowing/*.ui"
        "${src_path}/inspection/*.ui"
        "${src_path}/net/*.ui"
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
        Qt${QT_VERSION_MAJOR}::Svg
        ${VTK_LIBRARIES}
        #${VTK_TARGETS}
        ${OpenCV_LIBS}
        ${ITK_LIBRARIES}
        nlohmann_json::nlohmann_json
        yaml-cpp::yaml-cpp
        #cx_mct_recon
        CLreconstruct_d
        ACuda64D
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