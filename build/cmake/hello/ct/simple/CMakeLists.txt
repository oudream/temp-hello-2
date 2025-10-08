
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vcpkg toolchain 一般用命令行传；这里不写死
# -DCMAKE_TOOLCHAIN_FILE=D:/ct/vcpkg/scripts/buildsystems/vcpkg.cmake

find_package(Qt6 REQUIRED COMPONENTS Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 指定需要的 VTK 模块（含 Qt GUI 支持）
find_package(VTK REQUIRED COMPONENTS
  CommonCore
  CommonDataModel
  ImagingCore
  InteractionStyle
  RenderingCore
  RenderingOpenGL2
  RenderingVolumeOpenGL2
  RenderingVolume
  GUISupportQt
)

find_package(OpenCV REQUIRED)
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})       # ITK 的传统用法
include(${VTK_USE_FILE})       # 兼容旧宏；新模块系统也可用

add_executable(hello_ct_simple
        ${gs_project_hello_ct_path}/simple/main.cpp
        ${gs_project_hello_ct_path}/simple/awin.cpp
        ${gs_project_hello_ct_path}/simple/awin.h
        ${gs_project_hello_ct_path}/simple/awin.ui
)

target_link_libraries(hello_ct_simple PRIVATE
  Qt6::Widgets
  ${VTK_LIBRARIES}
  ${OpenCV_LIBS}
  ${ITK_LIBRARIES}
)

# VTK 9 模块自动初始化（某些配置下必须）
vtk_module_autoinit(TARGETS hello_ct_simple MODULES ${VTK_LIBRARIES})

# VTK 与 Qt 运行时初始化（必须）
if (VTK_VERSION VERSION_LESS "9.0.0")
    message(FATAL_ERROR "VTK >= 9 is required")
endif()