find_package(OpenCL REQUIRED)
if (OpenCL_FOUND)

    message("OpenCL found")
    set ( SIMPLEWORK_OPENCL_MODULE_NAME sw.device.opencl )
    set( SRC_LIST )
    file ( GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" )
    list ( APPEND SRC_LIST ${SRC_FILES} )
    
    add_library(${SIMPLEWORK_OPENCL_MODULE_NAME} SHARED ${SRC_LIST})
    set_target_properties(${SIMPLEWORK_OPENCL_MODULE_NAME} PROPERTIES PREFIX "")
    
    target_link_libraries(${SIMPLEWORK_OPENCL_MODULE_NAME} "sw.core")
    target_link_libraries(${SIMPLEWORK_OPENCL_MODULE_NAME} OpenCL::OpenCL)

else()
    message("OpenCL not found, doing something alternatively")
endif()
