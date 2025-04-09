cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

project("blitzbuffers-test")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(LINKER_LANGUAGE CXX)

add_executable(
	cpp_tests
	"test.cpp")

target_compile_features(cpp_tests PRIVATE cxx_std_20)

include(cmake/CPM.cmake)

# Add library to handle JSON files
CPMAddPackage(
	NAME nlohmann_json
	GITHUB_REPOSITORY nlohmann/json
	VERSION 3.11.3
	GIT_PROGRESS TRUE
  	OPTIONS
    	"JSON_BuildTests OFF"
)

target_link_libraries(cpp_tests nlohmann_json)
