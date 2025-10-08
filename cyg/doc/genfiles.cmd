@echo off
setlocal

REM === 1) 目錄 ===
for %%D in (
"ui"
"ui\app"
"ui\views"
"ui\panels"
"ui\pages"
"ui\dialogs"
"ui\adapters"
"model"
"model\objects"
"model\commands"
"model\bridge"
) do (
  if not exist "%%~D" mkdir "%%~D"
)

REM 小工具：創建空文件（若不存在則建立）
goto :defs

:mk
if not exist "%~1" (
  rem 建立空文件；可按需寫入頭註釋
  echo // TODO: implement > "%~1"
)
goto :eof

:defs

REM === 2) UI / app ===
call :mk "ui\app\app_ids.h"
call :mk "ui\app\u_app_window.h"
call :mk "ui\app\u_app_window.cpp"
call :mk "ui\app\u_ribbon.h"
call :mk "ui\app\u_ribbon.cpp"
call :mk "ui\app\u_status_bar.h"
call :mk "ui\app\u_status_bar.cpp"

REM === UI / views ===
for %%F in (
u_viewports_host
u_viewport_3d
u_viewport_axial
u_viewport_coronal
u_viewport_sagittal
u_overlay_measure
u_gizmo_knife
) do (
  call :mk "ui\views\%%F.h"
  call :mk "ui\views\%%F.cpp"
)

REM === UI / panels ===
for %%F in (
u_panel_knife
u_panel_render
u_panel_measure
u_panel_camera
u_panel_log
u_assets_panel
) do (
  call :mk "ui\panels\%%F.h"
  call :mk "ui\panels\%%F.cpp"
)

REM === UI / pages ===
for %%F in (
page_base
project_page
assets_page
reconstruct_page
inspect_page
thread_page
) do (
  call :mk "ui\pages\%%F.h"
  call :mk "ui\pages\%%F.cpp"
)

REM === UI / dialogs ===
for %%F in (
u_dialog_open
u_dialog_report
) do (
  call :mk "ui\dialogs\%%F.h"
  call :mk "ui\dialogs\%%F.cpp"
)

REM === UI / adapters ===
for %%F in (
u_event_adapter
u_command_helpers
) do (
  call :mk "ui\adapters\%%F.h"
  call :mk "ui\adapters\%%F.cpp"
)

REM === 3) MODEL / core files ===
for %%F in (
mtypes
mobject
medit_context
mrelation_index
mchange_tracker
mevent_hub
mcommand_bus
mscene
) do (
  call :mk "model\%%F.h"
  call :mk "model\%%F.cpp"
)

REM 單獨頭文件
call :mk "model\mevent.h"
call :mk "model\mcommand.h"

REM === MODEL / objects ===
for %%F in (
mvolume
mknife_plane
mslice_set
mrender_style
mcamera
mmeasure_item
) do (
  call :mk "model\objects\%%F.h"
  call :mk "model\objects\%%F.cpp"
)

REM === MODEL / commands ===
for %%F in (
cmd_move_knife_plane
cmd_set_knife_thickness
cmd_set_knife_lockaxis
cmd_set_window_level
cmd_set_transfer_function
cmd_orbit_camera
cmd_dolly_camera
cmd_reset_camera
cmd_create_measure
cmd_edit_measure
cmd_delete_measure
cmd_set_selection
cmd_set_active_object
cmd_set_link_cameras
cmd_set_link_windowlevel
) do (
  call :mk "model\commands\%%F.h"
  call :mk "model\commands\%%F.cpp"
)

REM === MODEL / bridge ===
for %%F in (
x_slice_executor
x_render_bridge
x_pick_bridge
) do (
  call :mk "model\bridge\%%F.h"
  call :mk "model\bridge\%%F.cpp"
)

echo.
echo [OK] 目錄與空文件已生成。
endlocal
