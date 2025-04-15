cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

project("blitzbuffers-benchmark")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(LINKER_LANGUAGE CXX)

add_executable(
	cpp_benchmarks
	"main.cpp"
)
target_compile_features(cpp_benchmarks PRIVATE cxx_std_20)

include(cmake/CPM.cmake)

# flatbuffers
CPMAddPackage(
	GITHUB_REPOSITORY google/flatbuffers
	VERSION 23.5.26
	GIT_PROGRESS TRUE
	OPTIONS
	"FLATBUFFERS_BUILD_TESTS NO"
	"FLATBUFFERS_BUILD_GRPCTEST NO"
)

flatbuffers_generate_headers(
	NAME flatbuffers
	TARGET FlatbuffersGeneratedHeaders
	SCHEMAS "${CMAKE_SOURCE_DIR}/flatbuffers.fbs"
	FLAGS --gen-object-api --reflect-names --gen-name-strings --gen-compare --rust
)

# alpaca
CPMAddPackage(
	GITHUB_REPOSITORY p-ranav/alpaca
	GIT_TAG 83a592f0c3807500f1aaf3b07fd48105a01e2780
	GIT_PROGRESS TRUE
	"ALPACA_BUILD_TESTS OFF"
	"ALPACA_BUILD_BENCHMARKS OFF"
	"ALPACA_BUILD_SAMPLES OFF"
)


target_link_libraries(cpp_benchmarks flatbuffers FlatbuffersGeneratedHeaders alpaca)

# Include blitzbuffers directory
target_include_directories(cpp_benchmarks PUBLIC "${CMAKE_SOURCE_DIR}/../../src/blitzbuffers/cpp")

# Setup google benchmark
CPMAddPackage(
	NAME benchmark
	GITHUB_REPOSITORY google/benchmark
	VERSION 1.8.3
	GIT_PROGRESS TRUE
	OPTIONS "BENCHMARK_ENABLE_TESTING Off"
)

if(benchmark_ADDED)
	# enable c++11 to avoid compilation errors
	set_target_properties(benchmark PROPERTIES CXX_STANDARD 11)
endif()

target_link_libraries(cpp_benchmarks benchmark)
