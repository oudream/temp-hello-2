
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

find_package(OpenCV REQUIRED)

add_executable(hello_windows_memory1 MACOSX_BUNDLE
        ${gs_project_hello_windows_path}/memory1/win_mem.h
        ${gs_project_hello_windows_path}/memory1/win_mem.cpp
        ${gs_project_hello_windows_path}/memory1/main.cpp
)

target_link_libraries(hello_windows_memory1 PRIVATE
        ${OpenCV_LIBS}
)
