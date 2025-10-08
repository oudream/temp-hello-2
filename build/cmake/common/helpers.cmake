
set(src_helpers_simple_headers
        filesystem_helper.h
        itk_helper.h
        log_helper.h
        vtk_helper.h
        qt_helper.h
)

set(src_helpers_simple_cpps
        filesystem_helper.cpp
        itk_helper.cpp
        log_helper.cpp
        vtk_helper.cpp
        qt_helper.cpp
)

mc_merge_file_path(${gs_project_helpers_path} "${src_helpers_simple_headers}" src_helpers_simple_header_filepaths)
mc_merge_file_path(${gs_project_helpers_path} "${src_helpers_simple_cpps}" src_helpers_simple_cpp_filepaths)
