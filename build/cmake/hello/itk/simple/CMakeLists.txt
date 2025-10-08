
add_executable(hello_itk_simple ${gs_project_hello_itk_path}/simple/main.cpp)

target_link_libraries(hello_itk_simple PRIVATE
        fmt::fmt
        ${ITK_LIBRARIES}
)
