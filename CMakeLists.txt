
cmake_minimum_required(VERSION 3.11.0)

project(CxxSimulator
  VERSION 0.1.0.0
  DESCRIPTION "A simulator project for Advanced C++ course."
  HOMEPAGE_URL "https://github.com/AronRubin"
)
set(CxxSimulator_VENDOR "Aron Rubin")
set(PROJECT_VENDOR "${CxxSimulator_VENDOR}")


set(CMAKE_DEBUG_POSTFIX d)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")


if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

include(FetchContent)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.6.1
)

FetchContent_GetProperties(nlohmann_json)
if(NOT nlohmann_json_POPULATED)
  FetchContent_Populate(nlohmann_json)
endif()

option(USE_CATCH2_TEST "Use catch2 instead of googletest." OFF)

if(USE_CATCH2_TEST)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v2.8.0
  )
  FetchContent_GetProperties(Catch2)
  if(NOT catch2_POPULATED)
    FetchContent_Populate(Catch2)
    add_subdirectory(${catch2_SOURCE_DIR} ${catch2_BINARY_DIR} EXCLUDE_FROM_ALL)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
  endif()
else(USE_CATCH2_TEST)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.8.1
  )

  FetchContent_GetProperties(googletest)
  if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
    # list(APPEND CMAKE_MODULE_PATH ${googletest_BINARY_DIR}/generated)
    # find_package(GTest)
  endif()
endif(USE_CATCH2_TEST)

enable_testing()

include(CheckIncludeFiles)
CHECK_INCLUDE_FILES(sys/prctl.h HAVE_SYS_PRCTL_H)

find_package(Threads REQUIRED)

# Add source to this project's executable.
add_library(CxxSimulator SHARED
  src/Simulator.cpp
  src/Model.cpp
)

target_sources(CxxSimulator PRIVATE src/Simulation_p.h)

target_sources(CxxSimulator PRIVATE
    include/CxxSimulator/Simulator.h
    include/CxxSimulator/Model.h
    include/CxxSimulator/Instance.h
    include/CxxSimulator/cpp_utils.h
)

target_include_directories(CxxSimulator PRIVATE include)
if(EXISTS ${nlohmann_json_SOURCE_DIR})
  target_include_directories(CxxSimulator PRIVATE ${nlohmann_json_SOURCE_DIR}/include)
endif()

target_link_libraries(CxxSimulator
  PUBLIC Threads::Threads
)

add_executable(CxxSimulatorExec
  src/CxxSimulatorExec.cpp
)
set_property(TARGET CxxSimulatorExec PROPERTY OUTPUT_NAME CxxSimulator)
target_include_directories(CxxSimulatorExec PRIVATE include)
target_link_libraries(CxxSimulatorExec CxxSimulator)

add_subdirectory(models/queuing)

include(CTest)

add_executable(CxxSimulatorTest src/CxxSimulatorTest.cpp)
target_include_directories(CxxSimulatorTest PRIVATE include)
target_link_libraries(CxxSimulatorTest PUBLIC CxxSimulator)
if(USE_CATCH2_TEST)
  target_link_libraries(CxxSimulatorTest PUBLIC Catch2::Catch2)
  target_compile_definitions(CxxSimulatorTest PUBLIC USE_CATCH2)
  include(Catch)
  catch_discover_tests(CxxSimulatorTest)
else(USE_CATCH2_TEST)
  include(GoogleTest)
  target_link_libraries(CxxSimulatorTest PUBLIC gtest_main gmock)
  gtest_discover_tests(CxxSimulatorTest)
endif(USE_CATCH2_TEST)


include(FeatureSummary)
feature_summary(WHAT ALL)

# include(DumpProps)
# print_target_properties(diva)
# include(DumpVariables)
# dump_all_variables()


