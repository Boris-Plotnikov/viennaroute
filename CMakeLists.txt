cmake_minimum_required(VERSION 3.16)
project(ViennaRoute VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(EMSCRIPTEN)
  add_compile_options(-fexceptions)
endif()

if(MSVC)
  add_compile_options(/W4 /utf-8)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(viennaroute_core
  src/text.cpp
  src/network.cpp
  src/station_search.cpp
  src/route_planner.cpp
  src/formatter.cpp
)
target_include_directories(viennaroute_core PUBLIC include)

add_executable(viennaroute src/main.cpp)
target_link_libraries(viennaroute PRIVATE viennaroute_core)
target_compile_definitions(viennaroute PRIVATE VIENNAROUTE_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

if(EMSCRIPTEN)
  add_executable(viennaroute_web wasm/bindings.cpp)
  target_link_libraries(viennaroute_web PRIVATE viennaroute_core)
  set_target_properties(viennaroute_web PROPERTIES SUFFIX ".mjs")
  target_link_options(viennaroute_web PRIVATE
    -lembind -fexceptions -O2
    -sMODULARIZE=1 -sEXPORT_ES6=1 -sEXPORT_NAME=createViennaRoute
    -sENVIRONMENT=web -sALLOW_MEMORY_GROWTH=1
    "--embed-file=${CMAKE_CURRENT_SOURCE_DIR}/data@/data")
endif()

option(VIENNAROUTE_BUILD_TESTS "Build unit tests" ON)
if(VIENNAROUTE_BUILD_TESTS)
  enable_testing()
  find_package(GTest QUIET)
  if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(googletest)
  endif()
  add_executable(viennaroute_tests
    tests/text_test.cpp
    tests/network_test.cpp
    tests/station_search_test.cpp
    tests/route_planner_test.cpp
  )
  target_link_libraries(viennaroute_tests PRIVATE viennaroute_core GTest::gtest_main)
  target_compile_definitions(viennaroute_tests PRIVATE VIENNAROUTE_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
  include(GoogleTest)
  gtest_discover_tests(viennaroute_tests)
endif()
