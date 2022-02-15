cmake_minimum_required (VERSION 3.16)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)

project(partix C)

message(STATUS "Setting policy CMP0074 to use <Package>_ROOT variables")
cmake_policy(SET CMP0074 NEW)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/modules")
message(STATUS "CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}")

set(PUBLIC_DEPS)
set(BACKEND_NAME)
set(PREFIX_BACKEND_SRC_PATH "threading")

option(Partix_ENABLE_QTHREADS "Whether to build with Qthreads" OFF)
option(Partix_ENABLE_PTHREADS "Whether to build with Pthreads" OFF)
option(Partix_ENABLE_ARGOBOTS "Whether to build with Argobots" OFF)
option(Partix_ENABLE_OPENMP "Whether to build with OpenMP" OFF)

find_package(MPI REQUIRED)
add_library(MPI INTERFACE)
list(APPEND MPI_C_LINK_FLAGS ${MPI_C_LIBRARIES})
set_target_properties(MPI PROPERTIES
  INTERFACE_COMPILE_OPTIONS "${MPI_C_COMPILE_FLAGS}"
  INTERFACE_INCLUDE_DIRECTORIES "${MPI_C_INCLUDE_PATH}"
  INTERFACE_LINK_LIBRARIES "${MPI_C_LINK_FLAGS}"
)

if (Partix_ENABLE_QTHREADS)
  set(BACKEND_NAME QTHREADS)
  set(BACKEND_LIB_NAME Qthreads)
  add_library(${BACKEND_LIB_NAME} INTERFACE)
  find_package(${BACKEND_LIB_NAME} REQUIRED)
  list(APPEND PUBLIC_DEPS ${BACKEND_LIB_NAME})
endif()
if (Partix_ENABLE_ARGOBOTS)
  set(BACKEND_NAME ARGOBOTS)  
  set(BACKEND_LIB_NAME Argobots)
  add_library(${BACKEND_LIB_NAME} INTERFACE)
  find_package(${BACKEND_LIB_NAME} REQUIRED)
  list(APPEND PUBLIC_DEPS ${BACKEND_LIB_NAME})
endif()
if (Partix_ENABLE_PTHREADS)
  set(BACKEND_NAME PTHREADS)
endif()
if (Partix_ENABLE_OPENMP)
  set(BACKEND_NAME OMP)
endif()

if (NOT BACKEND_NAME)
  message(FATAL_ERROR "Must give at least one valid backend.")
endif()

set(BACKEND_PATH)
set(SOURCE_PATH)
set(SOURCES)
set(HEADERS)

list(APPEND BACKEND_PATH "${PREFIX_BACKEND_SRC_PATH}")
list(APPEND BACKEND_PATH "${PREFIX_BACKEND_SRC_PATH}/impl/${BACKEND_NAME}")

foreach(DIR ${BACKEND_PATH})
  file(GLOB DIR_SRCS ${DIR}/*.c)
  file(GLOB DIR_HDRS ${DIR}/*.h)  
  list(APPEND SOURCES ${DIR_SRCS})
  list(APPEND HEADERS ${DIR_HDRS})
endforeach()

add_library(${BACKEND_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${BACKEND_NAME} PRIVATE ${PREFIX_BACKEND_SRC_PATH})
target_link_libraries(${BACKEND_NAME} PUBLIC ${PUBLIC_DEPS})

set(SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
set(HEADERS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/threading")
list(APPEND HEADERS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")

file(GLOB SOURCES ${SOURCE_PATH}/*.c)

foreach(DIR ${HEADERS_PATH})
  file(GLOB DIR_HDRS ${HEADERS_PATH}/*.h)  
  list(APPEND HEADERS ${DIR_HDRS})
endforeach()

foreach(SRC_FILE ${SOURCES})
  get_filename_component(SRC_FILE_NAME ${SRC_FILE} NAME)
  string(REGEX REPLACE "\\.[^.]*$" "" SRC_FILE_NAME ${SRC_FILE_NAME})
  add_executable(${SRC_FILE_NAME} ${SRC_FILE} ${HEADERS})
  target_include_directories(${SRC_FILE_NAME} PRIVATE ${HEADERS_PATH})
  target_link_libraries(${SRC_FILE_NAME} PUBLIC ${BACKEND_NAME})
endforeach()

