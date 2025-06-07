cmake_minimum_required(VERSION 3.15)
project(cpp-numeric-system VERSION 0.1.0 LANGUAGES CXX)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
enable_testing() # <<< ОБЯЗАТЕЛЬНО ДЛЯ CTEST

# Добавляем библиотеку
add_subdirectory(numeric-system)

option(GENERATE_TESTS "Build unit tests" OFF)
option(GENERATE_DOC "Generate Doxygen documentation" OFF)
option(GENERATE_BENCHMARK "Build the main executable 'benchmark'" OFF)

if(GENERATE_TESTS)
    add_subdirectory(tests)
endif()

if(GENERATE_DOC)
    add_subdirectory(docs)
endif()

if(GENERATE_BENCHMARK)
    add_subdirectory(benchmark)
endif()