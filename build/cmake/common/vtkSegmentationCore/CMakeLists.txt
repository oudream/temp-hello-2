project(vtkSegmentationCore LANGUAGES CXX)

# ------------------------------------------------------------
# Sources (explicitly listed)
# ------------------------------------------------------------
set(_HEADERS
        vtkBinaryLabelmapToClosedSurfaceConversionRule.h
        vtkCalculateOversamplingFactor.h
        vtkClosedSurfaceToBinaryLabelmapConversionRule.h
        vtkClosedSurfaceToFractionalLabelmapConversionRule.h
        vtkFractionalLabelmapToClosedSurfaceConversionRule.h
        vtkOrientedImageData.h
        vtkOrientedImageDataResample.h
        vtkPolyDataToFractionalLabelmapFilter.h
        vtkSegment.h
        vtkSegmentation.h
        vtkSegmentationConversionParameters.h
        vtkSegmentationConversionPath.h
        vtkSegmentationConverter.h
        vtkSegmentationConverterFactory.h
        vtkSegmentationConverterRule.h
        vtkSegmentationCoreConfigure.h         # from configure_file
        vtkSegmentationHistory.h
        vtkSegmentationModifier.h
        vtkTopologicalHierarchy.h
)

set(_SRCS
        vtkBinaryLabelmapToClosedSurfaceConversionRule.cxx
        vtkCalculateOversamplingFactor.cxx
        vtkClosedSurfaceToBinaryLabelmapConversionRule.cxx
        vtkClosedSurfaceToFractionalLabelmapConversionRule.cxx
        vtkFractionalLabelmapToClosedSurfaceConversionRule.cxx
        vtkOrientedImageData.cxx
        vtkOrientedImageDataResample.cxx
        vtkPolyDataToFractionalLabelmapFilter.cxx
        vtkSegment.cxx
        vtkSegmentation.cxx
        vtkSegmentationConversionParameters.cxx
        vtkSegmentationConversionPath.cxx
        vtkSegmentationConverter.cxx
        vtkSegmentationConverterFactory.cxx
        vtkSegmentationConverterRule.cxx
        vtkSegmentationHistory.cxx
        vtkSegmentationModifier.cxx
        vtkTopologicalHierarchy.cxx
)

mc_merge_file_path(${gs_project_vtkSegmentationCore_path} "${_HEADERS}" PROJECT_HEADERS)
mc_merge_file_path(${gs_project_vtkSegmentationCore_path} "${_SRCS}" PROJECT_SRCS)

# ------------------------------------------------------------
# Library
# ------------------------------------------------------------
add_library(${PROJECT_NAME} STATIC
        ${PROJECT_SRCS}
#        ${PROJECT_HEADERS}
)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        ${VTK_LIBRARIES}
)
