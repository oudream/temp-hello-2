
# cmake common func
include(utils.cmake)

## 开启 CTest 集成（会定义 BUILD_TESTING 选项）
#include(CTest)
## 显式启用（可选）
#enable_testing()


# ================== compile ==================
if(MSVC)
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
#    add_compile_options(/bigobj /utf-8)
else()
    add_compile_options(-fPIC)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 提供 GTest::gtest GTest::gtest_main 等
find_package(GTest CONFIG REQUIRED)

# 提供 benchmark::benchmark[_main]
find_package(benchmark CONFIG REQUIRED)

# qt
find_package(Qt6 COMPONENTS Core Gui Widgets Svg REQUIRED)

if (UNIX AND NOT APPLE AND ((QT_VERSION_MAJOR STREQUAL "6" AND QT_VERSION_MINOR GREATER 8) OR QT_VERSION_MAJOR STREQUAL "5"))
    include_directories(${Qt${QT_VERSION_MAJOR}Gui_PRIVATE_INCLUDE_DIRS})
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 指定需要的 VTK 模块（含 Qt GUI 支持）
find_package(VTK 9.0 REQUIRED COMPONENTS
        CommonCore
        CommonDataModel
        CommonExecutionModel
        FiltersCore
        FiltersGeneral
        FiltersGeometry
        FiltersParallel
        FiltersParallelDIY2
        ImagingCore
        ImagingStatistics
        InteractionImage
        InteractionStyle
        ImagingStencil
        InteractionWidgets
        RenderingCore
        RenderingOpenGL2
        RenderingVolume
        RenderingVolumeOpenGL2
        RenderingQt
        GUISupportQt
        RenderingCellGrid
        RenderingGL2PSOpenGL2
        IOXML
)
# 统一收集需要的 VTK 模块
set(VTK_TARGETS
        VTK::CommonCore
        VTK::CommonDataModel
        VTK::CommonExecutionModel
        VTK::FiltersCore
        VTK::FiltersGeneral
        VTK::FiltersGeometry
        VTK::FiltersParallel
        VTK::FiltersParallelDIY2
        VTK::ImagingCore
        VTK::ImagingStatistics
        VTK::ImagingStencil
        VTK::InteractionImage
        VTK::InteractionStyle
        VTK::InteractionWidgets
        VTK::RenderingCore
        VTK::RenderingOpenGL2
        VTK::RenderingVolume
        VTK::RenderingVolumeOpenGL2
        VTK::RenderingQt
        VTK::GUISupportQt
        VTK::RenderingCellGrid
        VTK::RenderingGL2PSOpenGL2
        VTK::IOXML
)

find_package(OpenCV REQUIRED)
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})       # ITK 的传统用法
# VTK 9.x 走“目标导向”写法，不再需要这个宏
#include(${VTK_USE_FILE})       # 兼容旧宏；新模块系统也可用

# json
find_package(nlohmann_json REQUIRED)

# yaml
find_package(yaml-cpp REQUIRED)


# common
set(gs_project_ccxx_path ${gs_project_common_path}/ccxx)

set(gs_project_helpers_path ${gs_project_common_path}/helpers)

set(gs_project_docking_path ${gs_project_common_path}/docking)

set(gs_project_vtkITK_path ${gs_project_common_path}/vtkITK)

set(gs_project_vtkSegmentationCore_path ${gs_project_common_path}/vtkSegmentationCore)

# include
include_directories(${gs_project_common_path})

# link
link_directories(${gs_project_deploy_path})

# docking
include(common/docking.cmake)

# helpers
include(common/helpers.cmake)

#
add_subdirectory(common)
#
add_subdirectory(gtest)
#
add_subdirectory(benchmark)
#
add_subdirectory(hello)

#add_subdirectory(tools)

add_subdirectory(demo)

add_subdirectory(cyg)
