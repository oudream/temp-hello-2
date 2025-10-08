
# ================== Path ==================
# business
set(gs_project_cyg_business_path ${gs_project_cyg_path}/business)

# gtest
set(gs_project_cyg_gtest_path ${gs_project_cyg_path}/gtest)

# bench
set(gs_project_cyg_bench_path ${gs_project_cyg_path}/benchmark)

# commands
set(gs_project_cyg_commands_path ${gs_project_cyg_path}/commands)

# tools
set(gs_project_cyg_tools_path ${gs_project_cyg_path}/tools)

# main
set(gs_project_cyg_main_path ${gs_project_cyg_path}/main)


# ================== Sub Dir ==================
add_subdirectory(gtest)

add_subdirectory(benchmark)

add_subdirectory(commands)

add_subdirectory(tools)

add_subdirectory(main)
