
macro(mc_create_bench1 sCxxName)
    add_executable(bench_${sCxxName}
            ${gs_project_ccxx_path}/${sCxxName}.cpp
    )

    # 链接 benchmark_main（自带 main()）
    target_link_libraries(bench_${sCxxName} PRIVATE
            ccxx
            benchmark::benchmark
            benchmark::benchmark_main
    )

    add_test(NAME bench.smoke COMMAND bench_${sCxxName} --benchmark_min_time=0.02 --benchmark_filter=Smoke)

    set_target_properties(bench_${sCxxName} PROPERTIES FOLDER benchmark)
endmacro()

mc_create_bench1(cxstring_bench)
