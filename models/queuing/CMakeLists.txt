
cmake_minimum_required(VERSION 3.11.0)

add_library(SimQueuing STATIC
  src/SimQueuing.cpp
)
target_include_directories(SimQueuing PRIVATE ${CMAKE_SOURCE_DIR}/include)
