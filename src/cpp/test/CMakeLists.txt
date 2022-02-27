cmake_minimum_required(VERSION 3.0.0)
project(SimpleWork VERSION 0.1.0)

find_package(OpenCL REQUIRED)

set ( SIMPLEWORK_TEST_MODULE_NAME sw.test )
file ( GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" )
set( SRC_LIST )
list ( APPEND SRC_LIST ${SRC_FILES} )
add_executable(${SIMPLEWORK_TEST_MODULE_NAME} ${SRC_LIST})
target_link_libraries(${SIMPLEWORK_TEST_MODULE_NAME} "sw.core" OpenCL::OpenCL)
#set_target_properties(${SIMPLEWORK_TEST_MODULE_NAME} PROPERTIES PREFIX "")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
