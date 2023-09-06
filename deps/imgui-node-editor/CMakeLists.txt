# Project definition
cmake_minimum_required(VERSION 3.1)
project(imgui-node-editor VERSION 1.0.0 LANGUAGES CXX)

# Source files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui-node-editor")
file(GLOB SOURCES LIST_DIRECTORIES false CONFIGURE_DEPENDS ${SRC_DIR}/*.cpp ${SRC_DIR}/*.c ${SRC_DIR}/*.hpp ${SRC_DIR}/*.h)

# Library definition and features
add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC "${SRC_DIR}")
target_link_libraries(${PROJECT_NAME} imgui)
