cmake_minimum_required(VERSION 3.0.0)
project(SimpleWork VERSION 0.1.0)

file ( GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/core/src/*.cpp" )
list ( APPEND LIB_SRC_LIST ${SRC_FILES} )

file ( GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/tensor/src/*.cpp" )
list ( APPEND LIB_SRC_LIST ${SRC_FILES} )

add_library(SimpleWork SHARED ${LIB_SRC_LIST})

file ( GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp" )
list ( APPEND TEST_SRC_LIST ${SRC_FILES} )

add_executable(SimpleWork_Test ${TEST_SRC_LIST})
target_link_libraries(SimpleWork_Test SimpleWork)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
