
# app

set(src_path ${gs_project_cyg_commands_path}/convert3d)

include_directories(${src_path})

add_executable(cx_mct_convert3d
        ${src_path}/main.cpp
)

#message("--- --- --- VTK_LIBRARIES: ${VTK_LIBRARIES}")

target_link_libraries(cx_mct_convert3d PRIVATE
        vtkITK
        ${VTK_LIBRARIES}
        ${ITK_LIBRARIES}
)

if (UNIX AND NOT APPLE)
    if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
        find_package(X11 REQUIRED)
        target_link_libraries(cx_mct_convert3d PUBLIC X11::xcb)
    else()
        target_link_libraries(cx_mct_convert3d PUBLIC xcb)
    endif()
endif()

vtk_module_autoinit(
        TARGETS cx_mct_convert3d
        MODULES ${VTK_LIBRARIES}
)

