cmake_minimum_required(VERSION 3.0.0)

set ( SIMPLEWORK_MATH_MODULE_NAME sw.math )
set( SRC_LIST )
file ( GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" )
list ( APPEND SRC_LIST ${SRC_FILES} )
file ( GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/operators/*.cpp" )
list ( APPEND SRC_LIST ${SRC_FILES} )

add_library(${SIMPLEWORK_MATH_MODULE_NAME} SHARED ${SRC_LIST})
target_link_libraries(${SIMPLEWORK_MATH_MODULE_NAME} "sw.core")
set_target_properties(${SIMPLEWORK_MATH_MODULE_NAME} PROPERTIES PREFIX "")

file( GLOB allOpenCLFiles "${CMAKE_CURRENT_SOURCE_DIR}/operators/*.cl")
file( COPY ${allOpenCLFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin/opencl/sw.math" )
