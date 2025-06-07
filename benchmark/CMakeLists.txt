set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(${EXTERNAL_DIR}/benchmark benchmark_build EXCLUDE_FROM_ALL)
set_property(TARGET benchmark PROPERTY FOLDER "external/benchmark")
set_property(TARGET benchmark_main PROPERTY FOLDER "external/benchmark")

file(GLOB_RECURSE BENCHMARK_SOURCES CONFIGURE_DEPENDS "*.cpp")

# Используем уникальное имя для исполняемого файла, чтобы не конфликтовать с библиотекой benchmark
set(NUMSYS_BENCHMARK_NAME benchmark_cpp_numeric_system)

add_executable(${NUMSYS_BENCHMARK_NAME} ${BENCHMARK_SOURCES})
target_link_libraries(${NUMSYS_BENCHMARK_NAME}
    PRIVATE
        ${PROJECT_NAME}
        benchmark::benchmark_main
)
set_property(TARGET ${NUMSYS_BENCHMARK_NAME} PROPERTY FOLDER "external/benchmark")
target_compile_options(${NUMSYS_BENCHMARK_NAME} PRIVATE
    $<$<CONFIG:Debug>:/Od>  
    $<$<CONFIG:Release>:/O2>
)


add_test(NAME rbenchmark COMMAND $<TARGET_FILE:${NUMSYS_BENCHMARK_NAME}>
    --benchmark_out=${CMAKE_BINARY_DIR}/rbenchmark.json
    --benchmark_out_format=json
)
