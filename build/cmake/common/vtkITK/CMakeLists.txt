
project(vtkITK LANGUAGES CXX)

# ------------------------------------------------------------------------------
# Source files
# ------------------------------------------------------------------------------
set(_SRCS
        FIBHEAP.cxx
        FIBHEAP.h
        GrowCutExport.h
        vtkITKNumericTraits.cxx
        vtkITKArchetypeDiffusionTensorImageReaderFile.cxx
        vtkITKArchetypeImageSeriesReader.cxx
        vtkITKArchetypeImageSeriesScalarReader.cxx
        vtkITKArchetypeImageSeriesVectorReaderFile.cxx
        vtkITKArchetypeImageSeriesVectorReaderSeries.cxx
        vtkITKImageThresholdCalculator.cxx
        vtkITKImageWriter.cxx
        vtkITKGradientAnisotropicDiffusionImageFilter.cxx
        vtkITKGrowCut.cxx
        vtkITKDistanceTransform.cxx
        vtkITKLabelShapeStatistics.cxx
        vtkITKLevelTracingImageFilter.cxx
        vtkITKLevelTracing3DImageFilter.cxx
        vtkITKIslandMath.cxx
        vtkITKImageMargin.cxx
        vtkITKMorphologicalContourInterpolator.cxx
        vtkITKImageSequenceReader.cxx
        vtkITKImageSequenceWriter.cxx
)

set(_HEADERS
        GrowCutExport.h
        FIBHEAP.h
        GrowCut.h
        itkFastGrowCut.h
        itkFastGrowCut.hxx
        itkLevelTracingImageFilter.h
        itkLevelTracingImageFilter.txx
        itkMorphologicalContourInterpolator.h
        itkMorphologicalContourInterpolator.hxx
        vtkITKImageToImageFilter.h
        vtkITKImageToImageFilterFF.h
        vtkITKImageToImageFilter2DFF.h
        vtkITKImageToImageFilterFUL.h
        vtkITKImageToImageFilterSS.h
        vtkITKImageToImageFilterULUL.h
        vtkITKImageToImageFilterUSUL.h
        vtkITKImageToImageFilterUSUS.h
        vtkITKImageToImageFilterUSF.h
        vtkITKConfigure.h
        GrowCutExport.h
)

mc_merge_file_path(${gs_project_vtkITK_path} "${_HEADERS}" PROJECT_HEADERS)
mc_merge_file_path(${gs_project_vtkITK_path} "${_SRCS}" PROJECT_SRCS)

# ------------------------------------------------------------------------------
# Library target
# ------------------------------------------------------------------------------
add_library(${PROJECT_NAME} STATIC
        ${PROJECT_SRCS}
#        ${PROJECT_HEADERS}
)

# ------------------------------------------------------------------------------
# Link libraries
# ------------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME}
        PUBLIC
        ${VTK_LIBRARIES}
        ${ITK_LIBRARIES}
)
