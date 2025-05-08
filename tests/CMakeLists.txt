find_package(GTest REQUIRED)
enable_testing()

message(STATUS "CMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD},
    third_libraries=${THIRD_LIBRARIES}")
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/gtest/*.cc)
message(STATUS "gtests sources: ${SOURCES}")
add_executable(test_main ${SOURCES})
target_link_libraries(test_main ${PROJECT_NAME} GTest::GTest GTest::Main)
