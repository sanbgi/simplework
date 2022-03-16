cmake_minimum_required(VERSION 3.0.0)

#add_subdirectory(cuda)
add_subdirectory(opencl)

set ( SIMPLEWORK_DEVICE_MODULE_NAME sw.device )
set( SRC_LIST )
file ( GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" )
list ( APPEND SRC_LIST ${SRC_FILES} )
file ( GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/cpu/*.cpp" )
list ( APPEND SRC_LIST ${SRC_FILES} )
file ( GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/operators/*.cpp" )
list ( APPEND SRC_LIST ${SRC_FILES} )
add_library(${SIMPLEWORK_DEVICE_MODULE_NAME} SHARED ${SRC_LIST})
set_target_properties(${SIMPLEWORK_DEVICE_MODULE_NAME} PROPERTIES PREFIX "")

target_link_libraries(${SIMPLEWORK_DEVICE_MODULE_NAME} "sw.core")

file( GLOB allOpenCLFiles "${CMAKE_CURRENT_SOURCE_DIR}/operators/*.cl")
file( COPY ${allOpenCLFiles} DESTINATION "${PROJECT_BINARY_DIR}/bin/opencl/sw.device" )
