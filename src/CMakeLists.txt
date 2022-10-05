cmake_minimum_required(VERSION 3.10)
project(${PROJECT_NAME})    

message("Building STATIC Library")

set (SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/glad.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/ini.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/INIReader.cpp")

target_sources(${PROJECT_NAME} PUBLIC ${SRC_FILES})