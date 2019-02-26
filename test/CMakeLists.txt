cmake_minimum_required(VERSION 2.6)

# --- Coverage specific configuration --- start
# required: Coverage or Debug
# (alternativelly via command line option: cmake -DCMAKE_BUILD_TYPE=Coverage ..)
set(CMAKE_BUILD_TYPE Coverage)
# add the project specific cmake modules directory to the cmake module path
# (w.r.t. CI/CD builds this is not the best approach)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
# include the cmake code coverage module
include(CodeCoverage)
# test coverage build configuration for C++: debug build, no optimization, profiling enabled
set(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")

# exclude /test and /ext directories from coverage analysis
set(LCOV_REMOVE_EXTRA "'mock_libs/*'" "'unit_test/*'" "'ext/*'")

# cmake code coverage module target
setup_target_for_coverage(unit_tests_coverage unit_tests coverage)

# --- Coverage specific configuration --- end


set(catch_includes ${PROJECT_SOURCE_DIR}/ext)
set(includes_production ${PROJECT_SOURCE_DIR}/../zoho-iot-sdk/)
include_directories(${catch_includes} ${includes_production})

include_directories(
                    ${PROJECT_SOURCE_DIR}/lib/ArduinoJson/src/ 
    	            ${PROJECT_SOURCE_DIR}/mock_libs/
                    )

file(GLOB_RECURSE sources_production ${PROJECT_SOURCE_DIR}/../zoho-iot-sdk/*.c*)
file(GLOB_RECURSE sources_test unit_test/*.c*)
add_executable(unit_tests ${sources_test} ${sources_production})

target_include_directories (unit_tests
	PUBLIC ${PROJECT_SOURCE_DIR}/ext
	PUBLIC ${PROJECT_SOURCE_DIR}/lib/fakeit_master/single_header/catch
)

list(   APPEND paho_srcs 
        "${PROJECT_SOURCE_DIR}/mock_libs/PubSubClient.cpp" 
        "${PROJECT_SOURCE_DIR}/mock_libs/PubSubClient.h" )

ADD_LIBRARY(pubsub_lib STATIC ${paho_srcs})

target_link_libraries(unit_tests pubsub_lib)

