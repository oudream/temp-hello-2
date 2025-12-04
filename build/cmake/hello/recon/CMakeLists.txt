
set(gs_project_hello_recon_path ${gs_project_hello_path}/recon)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



## ---- DLL（SHARED 库）----
#add_library(cx_mct_recon SHARED
#        ${gs_project_hello_recon_path}/ct_recon_impl.c
#)
## 定义导出宏（Windows 下 __declspec(dllexport)）
#target_compile_definitions(cx_mct_recon PRIVATE RECON_BUILD_DLL)
#
## 可选：更严格的警告
#if (MSVC)
#    target_compile_options(cx_mct_recon PRIVATE /W4)
#else()
#    target_compile_options(cx_mct_recon PRIVATE -Wall -Wextra -Wpedantic)
#endif()



# ---- C++ 门面 + 示例 ----
add_executable(hello_ct_recon_example
        ${gs_project_hello_recon_path}/sample_main.cpp
)
target_link_libraries(hello_ct_recon_example PRIVATE cx_mct_recon)
