
# Prevent a "command line is too long" failure in Windows.
add_executable(hello_vtk_simple MACOSX_BUNDLE ${gs_project_hello_vtk_path}/simple/CylinderExample.cxx )

#message("--- --- --- VTK_LIBRARIES" ${VTK_LIBRARIES})

set(VTK_MODULES
        VTK::CommonColor
        VTK::CommonComputationalGeometry
        VTK::CommonCore
        VTK::CommonDataModel
        VTK::CommonExecutionModel
        VTK::CommonTransforms
        VTK::FiltersCore
        VTK::FiltersGeneral
        VTK::FiltersHybrid
        VTK::FiltersModeling
        VTK::FiltersSources
        VTK::IOImage
        VTK::IOXML
        VTK::InteractionStyle
        VTK::InteractionWidgets
        VTK::RenderingAnnotation
        VTK::RenderingCore
        VTK::RenderingFreeType
        VTK::RenderingLabel
        VTK::RenderingOpenGL2
        VTK::fmt
        VTK::TestingRendering
)

target_link_libraries(hello_vtk_simple PRIVATE ${VTK_MODULES})

vtk_module_autoinit(
        TARGETS hello_vtk_simple
        MODULES ${VTK_MODULES}
)
#
#target_link_libraries(hello_vtk_simple PRIVATE
#        CommonColor
#        CommonComputationalGeometry
#        CommonCore
#        CommonDataModel
#        CommonTransforms
#        FiltersCore
#        FiltersGeneral
#        FiltersSources
#        IOParallel
#        InteractionStyle
#        InteractionWidgets
#        RenderingAnnotation
#        RenderingCore
#        RenderingFreeType
#        RenderingOpenGL2
##        OPTIONAL_COMPONENTS
#        TestingRendering
#)
