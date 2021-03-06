cmake_minimum_required(VERSION 3.0.0)
#add_subdirectory("thirdpart")

set ( SIMPLEWORK_AV_MODULE_NAME sw.av )
file ( GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" )
file ( GLOB FFMPEG_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/*.cpp" )
file ( GLOB SDL_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sdl/*.cpp" )
file ( GLOB PIPE_SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/pipe/*.cpp" )
set( SRC_LIST )
list ( APPEND SRC_LIST ${SRC_FILES} )
list ( APPEND SRC_LIST ${FFMPEG_SRC_FILES} )
list ( APPEND SRC_LIST ${SDL_SRC_FILES} )
list ( APPEND SRC_LIST ${PIPE_SRC_FILES} )
add_library(${SIMPLEWORK_AV_MODULE_NAME} SHARED ${SRC_LIST})

target_link_libraries(${SIMPLEWORK_AV_MODULE_NAME} "sw.core")
target_include_directories(${SIMPLEWORK_AV_MODULE_NAME} PRIVATE "../inc")

find_package(SDL2 REQUIRED)
target_link_libraries(${SIMPLEWORK_AV_MODULE_NAME} ${SDL2_LIBRARIES})
target_include_directories(${SIMPLEWORK_AV_MODULE_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})

find_package(FFMPEG REQUIRED)
target_link_libraries(${SIMPLEWORK_AV_MODULE_NAME}  ${FFMPEG_LIBRARIES})
target_include_directories(${SIMPLEWORK_AV_MODULE_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})

find_package(JPEG REQUIRED)
target_link_libraries(${SIMPLEWORK_AV_MODULE_NAME} ${JPEG_LIBRARIES})
target_include_directories(${SIMPLEWORK_AV_MODULE_NAME} PRIVATE ${JPEG_INCLUDE_DIR})

#target_link_libraries(${SIMPLEWORK_AV_MODULE_NAME} "jpeg")
#target_include_directories(${SIMPLEWORK_AV_MODULE_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/thirdpart/jpeg "thirdpart/jpeg")

set_target_properties(${SIMPLEWORK_AV_MODULE_NAME} PROPERTIES PREFIX "")
message(STATUS "JPEG_INCLUDE_DIR: ${JPEG_INCLUDE_DIR}")
message(STATUS "JPEG_LIBRARIES: ${JPEG_LIBRARIES}")
message(STATUS "FFMPEG_INCLUDE_DIRS: ${FFMPEG_INCLUDE_DIRS}")
message(STATUS "FFMPEG_LIBRARIES: ${FFMPEG_LIBRARIES}")
message(STATUS "SDL2_INCLUDE_DIRS: ${FFMPEG_INCLUDE_DIRS}")
message(STATUS "SDL2_LIBRARIES: ${SDL2_LIBRARIES}")

