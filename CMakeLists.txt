cmake_minimum_required(VERSION 3.30)
project(template-benchmark LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(benchmark REQUIRED)
find_package(frozenchars CONFIG REQUIRED)
find_package(glaze CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

find_path(INJA_INCLUDE_DIR "inja/inja.hpp")
if(NOT INJA_INCLUDE_DIR)
  message(FATAL_ERROR "inja not found")
endif()

set(COMMON_LIBS
  benchmark::benchmark
  benchmark::benchmark_main
  frozenchars::frozenchars
  glaze::glaze
  nlohmann_json::nlohmann_json
)

macro(add_benchmark name)
  add_executable(${name} bench/${name}.cpp)
  target_include_directories(${name} PRIVATE include ${INJA_INCLUDE_DIR})
  target_link_libraries(${name} PRIVATE ${COMMON_LIBS})
  target_compile_features(${name} PRIVATE cxx_std_26)
endmacro()

add_benchmark(bench_html)
add_benchmark(bench_csv)
add_benchmark(bench_url)
add_benchmark(bench_json)
add_benchmark(bench_config)
add_benchmark(bench_markdown)
