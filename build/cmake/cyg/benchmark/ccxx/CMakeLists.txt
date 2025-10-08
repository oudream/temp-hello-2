
macro(mc_create_bench1 sCxxName)
    aux_source_directory(${gs_project_cyg_bench_path}/ccxx/${sCxxName} gsl_${sCxxName}_DIR_SRCS)

    add_executable(bench_cyg_${sCxxName} ${gsl_${sCxxName}_DIR_SRCS})

    # 链接 benchmark_main（自带 main()）
    target_link_libraries(bench_cyg_${sCxxName} PRIVATE
            ccxx
            benchmark::benchmark
            benchmark::benchmark_main
    )

    add_test(NAME bench.smoke COMMAND bench_cyg_${sCxxName} --benchmark_min_time=0.02 --benchmark_filter=Smoke)

    set_target_properties(bench_cyg_${sCxxName} PROPERTIES FOLDER benchmark)
endmacro()


mc_create_bench1(cxstring)
