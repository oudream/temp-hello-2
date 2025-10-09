
add_executable(demo_app1
        ${gs_project_demo_path}/app1/main.cpp
        ${gs_project_demo_path}/app1/app_config.cpp
        # helpers
        ${src_helpers_simple_header_filepaths}
        ${src_helpers_simple_cpp_filepaths}
)

target_link_libraries(demo_app1 PRIVATE
        ccxx
        vtkITK
        vtkSegmentationCore
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        ${VTK_LIBRARIES}
        ${OpenCV_LIBS}
        ${ITK_LIBRARIES}
        nlohmann_json::nlohmann_json
        yaml-cpp::yaml-cpp
)

if(QT_VERSION_MAJOR STREQUAL "6")
    target_link_libraries(demo_app1 PRIVATE Qt6::GuiPrivate) #needed for <qpa/qplatformnativeinterface.h>
endif()

if (UNIX AND NOT APPLE)
    if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
        find_package(X11 REQUIRED)
        target_link_libraries(demo_app1 PUBLIC X11::xcb)
    else()
        target_link_libraries(demo_app1 PUBLIC xcb)
    endif()
endif()

vtk_module_autoinit(
        TARGETS demo_app1
        MODULES ${VTK_LIBRARIES}
)
