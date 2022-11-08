cmake_minimum_required(VERSION 3.10)

########### Variables ############
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

set(PROJECT_NAME "PetForDesktop")
set(REL_RESOURCES_DIR "content")

set(ABS_RESOURCE_DIR "${CMAKE_SOURCE_DIR}/${REL_RESOURCES_DIR}")
set(ABS_DEPS_DIR "${CMAKE_SOURCE_DIR}/deps")
set(ABS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(ABS_SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(ABS_MAIN_FILE "${ABS_SRC_DIR}/main.cpp")
set(ABS_RESOURCES_BUILD_LINK "${CMAKE_CURRENT_BINARY_DIR}/${REL_RESOURCES_DIR}")
set(ABS_LICENSE_PATH "${CMAKE_SOURCE_DIR}/LICENSE")
set(ABS_README_PATH "${CMAKE_SOURCE_DIR}/README.md")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

########### Option ############
option(INSTALLER "Enable installer setting for build" OFF)

########### Create project ############
set(PROJECT_VERSION "1.0.0")
project(${PROJECT_NAME} VERSION "${PROJECT_VERSION}")

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

########### Build ############
file(GLOB_RECURSE project_source_files    LIST_DIRECTORIES false CONFIGURE_DEPENDS src/*.cpp src/*.c)
file(GLOB_RECURSE project_headers  LIST_DIRECTORIES false CONFIGURE_DEPENDS include/*.hpp include/*.h)
message(STATUS "Headers: ${project_headers}")
message(STATUS "Sources : ${project_source_files}")

# Set include directory
include_directories("${ABS_INCLUDE_DIR}/")

# add the executable
add_executable(${PROJECT_NAME} ${SUB_SYS} ${project_headers} ${project_source_files})

########### Install setup ############
# Setup install process
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR}/)

if (WIN32)
  install(DIRECTORY ${REL_RESOURCES_DIR} DESTINATION ${CMAKE_INSTALL_DATADIR}/)
else()
  install(DIRECTORY ${REL_RESOURCES_DIR} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR})
endif()

########### Installer ############
# add CPack to project
set(CPACK_GENERATOR WIX)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Jonathan Six")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "A pet for desktop like you've never dreamed 💚")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_NSIS_IGNORE_LICENSE_PAGE 1)
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})

set_property(
    INSTALL
        bin/$<TARGET_FILE_NAME:${PROJECT_NAME}>
    PROPERTY
        CPACK_START_MENU_SHORTCUTS
            ${PROJECT_NAME}
)

set_property(
    INSTALL
        bin/$<TARGET_FILE_NAME:${PROJECT_NAME}>
    PROPERTY
        CPACK_DESKTOP_SHORTCUTS
            ${PROJECT_NAME}
)

include(CPack)
set(CPACK_WIX_PROGRAM_MENU_FOLDER ${PROJECT_NAME})

########### Deps ############
# GLFW
set (GLFW_BUILD_EXAMPLES OFF)
set (GLFW_BUILD_TESTS OFF)
set (GLFW_BUILD_DOCS OFF)
set (GLFW_INSTALL OFF)
add_subdirectory(${ABS_DEPS_DIR}/glfw)
target_link_libraries(${PROJECT_NAME} glfw)
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} glfw )

# Boxer
add_subdirectory(${ABS_DEPS_DIR}/Boxer)
target_link_libraries(${PROJECT_NAME} Boxer)

# yaml-cpp
add_subdirectory(${ABS_DEPS_DIR}/yaml-cpp)
target_link_libraries(${PROJECT_NAME} yaml-cpp)

########### Define ############
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")

if (INSTALLER)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RESOURCE_PATH="${REL_RESOURCES_DIR}/")
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE RESOURCE_PATH="../${REL_RESOURCES_DIR}/")
endif()

if (WIN32)
  install(DIRECTORY ${REL_RESOURCES_DIR} DESTINATION ${CMAKE_INSTALL_DATADIR}/)
else()
  install(DIRECTORY ${REL_RESOURCES_DIR} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR})
endif()

########### Build link ############
# Set a symbolic link so exe can find the /Data folder
add_custom_command( 
    TARGET ${PROJECT_NAME} 
    POST_BUILD COMMAND ${CMAKE_COMMAND} 
        -E create_symlink ${ABS_RESOURCE_DIR} ${ABS_RESOURCES_BUILD_LINK} 
    DEPENDS ${ABS_RESOURCES_BUILD_LINK} 
    COMMENT "symbolic link ${ABS_RESOURCE_DIR} => ${ABS_RESOURCES_BUILD_LINK}" 
)