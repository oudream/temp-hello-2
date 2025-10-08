
add_executable(hello_yml_simple ${gs_project_hello_yml_path}/simple/main.cpp)

target_link_libraries(hello_yml_simple PRIVATE
        yaml-cpp::yaml-cpp
)
