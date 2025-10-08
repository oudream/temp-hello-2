
set(src_docking_headers
        ads_globals.h
        DockAreaTabBar.h
        DockAreaTitleBar.h
        DockAreaTitleBar_p.h
        DockAreaWidget.h
        DockContainerWidget.h
        DockManager.h
        DockOverlay.h
        DockSplitter.h
        DockWidget.h
        DockWidgetTab.h
        DockingStateReader.h
        DockFocusController.h
        ElidingLabel.h
        FloatingDockContainer.h
        FloatingDragPreview.h
        IconProvider.h
        DockComponentsFactory.h
        AutoHideSideBar.h
        AutoHideTab.h
        AutoHideDockContainer.h
        PushButton.h
        ResizeHandle.h
)

set(src_docking_cpps
        ads_globals.cpp
        DockAreaTabBar.cpp
        DockAreaTitleBar.cpp
        DockAreaWidget.cpp
        DockContainerWidget.cpp
        DockManager.cpp
        DockOverlay.cpp
        DockSplitter.cpp
        DockWidget.cpp
        DockWidgetTab.cpp
        DockingStateReader.cpp
        DockFocusController.cpp
        ElidingLabel.cpp
        FloatingDockContainer.cpp
        FloatingDragPreview.cpp
        IconProvider.cpp
        DockComponentsFactory.cpp
        AutoHideSideBar.cpp
        AutoHideTab.cpp
        AutoHideDockContainer.cpp
        PushButton.cpp
        ResizeHandle.cpp
        ads.qrc
)

mc_merge_file_path(${gs_project_docking_path} "${src_docking_headers}" src_docking_header_filepaths)
mc_merge_file_path(${gs_project_docking_path} "${src_docking_cpps}" src_docking_cpp_filepaths)
