cmake_minimum_required(VERSION 3.10)

########### Variables ############
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

set(PROJECT_NAME "PetDesktop")
set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/resources")
set(DEPS_DIR "${CMAKE_SOURCE_DIR}/deps")
set(INC_DIR "${CMAKE_SOURCE_DIR}/include")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(MAIN_FILE "${SRC_DIR}/main.cpp")
set(RESOURCE_LINK "${CMAKE_CURRENT_BINARY_DIR}/resources") 

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

########### Create project ############
project(${PROJECT_NAME})    

########### Setup ############
# Set env variable
IF(CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "Generate CMake cache for Release")
    IF(WIN32)
        set(SUB_SYS WIN32)
    ENDIF(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
ELSE()
    message(STATUS "Generate CMake cache for Debug")
    add_definitions(-DDEBUG)
ENDIF(CMAKE_BUILD_TYPE MATCHES Release)

########### Link ############
# Set include directory
include_directories("${INC_DIR}/")

# add the executable
add_executable(${PROJECT_NAME} ${SUB_SYS} ${MAIN_FILE})

# Add sub projects.
add_subdirectory(${SRC_DIR})

# GLFW
add_subdirectory(${DEPS_DIR}/glfw)
target_link_libraries(${PROJECT_NAME} glfw)
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} glfw )

########### Install build deps ############
# Set a symbolic link so exe can find the /Data folder
add_custom_command( 
    TARGET ${PROJECT_NAME} 
    POST_BUILD COMMAND ${CMAKE_COMMAND} 
        -E create_symlink ${RESOURCE_DIR} ${RESOURCE_LINK} 
    DEPENDS ${RESOURCE_LINK} 
    COMMENT "symbolic link ${RESOURCE_DIR} => ${RESOURCE_LINK}" 
)
#file(COPY ${RESOURCE_DIR} DESTINATION ${CMAKE_BINARY_DIR})