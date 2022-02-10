cmake_minimum_required(VERSION 3.12)
project(examples C)

# You need this for using Kokkos_ROOT variable
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.12.0")
  message(STATUS "Setting policy CMP0074 to use <Package>_ROOT variables")
  cmake_policy(SET CMP0074 NEW)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/modules")
message(STATUS "CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}")

set(PUBLIC_DEPS)

add_library(QTHREADS INTERFACE)
find_package(Qthreads REQUIRED)
list(APPEND PUBLIC_DEPS QTHREADS)

add_executable(example1 example1.c)
add_executable(example2 example2.c)
add_executable(example3 example3.c)
add_executable(example4 example4.c)

target_link_libraries(example1 PUBLIC ${PUBLIC_DEPS})
target_link_libraries(example2 PUBLIC ${PUBLIC_DEPS})
target_link_libraries(example3 PUBLIC ${PUBLIC_DEPS})
target_link_libraries(example4 PUBLIC ${PUBLIC_DEPS})
