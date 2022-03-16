cmake_minimum_required(VERSION 3.0.0)

find_package(CUDA REQUIRED)
if (CUDA_FOUND)

    message("CUDA found")
    message("CUDA_LIBRARIES:${CUDA_LIBRARIES}")
    set ( SIMPLEWORK_CUDA_MODULE_NAME sw.device.cuda )
    
    set( SRC_LIST )
    file ( GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" )
    list ( APPEND SRC_LIST ${SRC_FILES} )

    add_library(${SIMPLEWORK_CUDA_MODULE_NAME} SHARED ${SRC_LIST})
    set_target_properties(${SIMPLEWORK_CUDA_MODULE_NAME} PROPERTIES PREFIX "")

    target_link_libraries(${SIMPLEWORK_CUDA_MODULE_NAME} "sw.core")
    target_link_libraries(${SIMPLEWORK_CUDA_MODULE_NAME} ${CUDA_LIBRARIES} "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.6/lib/x64/cuda.lib" )
    include_directories(${SIMPLEWORK_CUDA_MODULE_NAME} PRIVARE ${CUDA_INCLUDE_DIRS})
    
else()
    message("CUDA not found, doing something alternatively")
endif()
