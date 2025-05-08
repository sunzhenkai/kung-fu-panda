cmake_minimum_required(VERSION 3.20)
project(kung-fu-panda)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(BUILD_TESTING "build unittests" OFF)
add_definitions(-Wall)

include(GNUInstallDirs) # CMAKE_INSTALL_INCLUDEDIR / CMAKE_INSTALL_LIBDIR / ...
include_directories(src)
if(BUILD_TESTING)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/build/install/)
endif()

# find
find_package(fmt REQUIRED)
find_package(absl REQUIRED)
find_package(spdlog REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(unofficial-brpc CONFIG REQUIRED)
find_package(cppcommon CONFIG REQUIRED)
find_package(kung-fu-panda-protocols CONFIG REQUIRED)
set(THIRD_LIBRARIES
    fmt::fmt spdlog::spdlog rapidjson
    kung-fu-panda-protocols::kung-fu-panda-protocols cppcommon::cppcommon
    unofficial::brpc::brpc-static)

file(GLOB_RECURSE LIB_HEADERS src/*.h)
file(GLOB_RECURSE LIB_SRCS src/*.cc)

add_library(${PROJECT_NAME} ${LIB_HEADERS} ${LIB_SRCS})
target_link_libraries(${PROJECT_NAME} ${THIRD_LIBRARIES})

add_executable(kfpanda cmd/main.cc)
target_link_libraries(kfpanda ${PROJECT_NAME})

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
