
set(gs_bench_stl_path ${gs_project_benchmark_path}/stl)

add_executable(bench_stl_random
        ${gs_bench_stl_path}/random/main.cpp
)

# 链接 benchmark_main（自带 main()）
target_link_libraries(bench_stl_random PRIVATE
        benchmark::benchmark
        benchmark::benchmark_main
)

# 可选：把一个“冒烟”基准挂到 ctest（避免跑太久）
add_test(NAME bench.smoke COMMAND bench_stl_random --benchmark_min_time=0.02 --benchmark_filter=Smoke)
