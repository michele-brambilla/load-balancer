cmake_minimum_required(VERSION 2.8.11)
project("kafka-to-nexus")

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake")
find_package(Googletest REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ..)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ..)

if (NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Debug)
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -fPIC -g -D_GLIBCXX_USE_NANOSLEEP")
if (DEFINED _GLIBCXX_USE_CXX11_ABI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=${_GLIBCXX_USE_CXX11_ABI}")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11 -O0 -fno-inline -ggdb -D_GLIBCXX_USE_NANOSLEEP -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11 -O3 -D_GLIBCXX_USE_NANOSLEEP")

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9.0")
    message(FATAL_ERROR "GCC below 4.9.0 does not fully support c++11.")
  endif()
endif()

if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9.3")
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=auto")
endif()

set(path_include_common
  ${PROJECT_SOURCE_DIR}
  )

set(tgt "tests")
set(sources
test.cxx
balancer.cxx
)
add_executable(${tgt} ${sources})
target_compile_definitions(${tgt} PRIVATE ${compile_defs_common})
target_include_directories(${tgt} PRIVATE ${path_include_common})
add_gtest_to_target(${tgt})
