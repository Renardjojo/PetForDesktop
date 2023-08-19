# From https://gist.github.com/rokups/f771217b2d530d170db5cb1e08e9a8f4
cmake_minimum_required(VERSION 3.10)

########### Variables ############
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

set(PROJECT_NAME "PetForDesktop")
set(PROJECT_VERSION 1.3.1)
set(REL_RESOURCES_DIR "content")

set(ABS_RESOURCE_DIR "${CMAKE_SOURCE_DIR}/${REL_RESOURCES_DIR}")
set(ABS_DEPS_DIR "${CMAKE_SOURCE_DIR}/deps")
set(ABS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(ABS_SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(ABS_MAIN_FILE "${ABS_SRC_DIR}/main.cpp")
set(ABS_RESOURCES_BUILD_LINK "${CMAKE_CURRENT_BINARY_DIR}/${REL_RESOURCES_DIR}")
set(ABS_LICENSE_PATH "${CMAKE_SOURCE_DIR}/LICENSE")
set(ABS_README_PATH "${CMAKE_SOURCE_DIR}/README.md")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

########### Create version ############
#find_package(Git)
#
#if(GIT_EXECUTABLE)
#  # Generate a git-describe version string from Git repository tags
#  execute_process(
#    COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --match "v*"
#    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#    OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
#    RESULT_VARIABLE GIT_DESCRIBE_ERROR_CODE
#    OUTPUT_STRIP_TRAILING_WHITESPACE
#    )
#
#  if(NOT GIT_DESCRIBE_ERROR_CODE)
#    string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${GIT_DESCRIBE_VERSION}")
#    string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${GIT_DESCRIBE_VERSION}")
#    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${GIT_DESCRIBE_VERSION}")
#    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" VERSION_SHA1 "${GIT_DESCRIBE_VERSION}")
#    set(PROJECT_VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
#    set(PROJECT_VERSION_SHA ${GIT_DESCRIBE_VERSION})
#  endif()
#endif()
#
## Final fallback: Just use a bogus version string that is semantically older
## than anything else and spit out a warning to the developer.
#if(NOT DEFINED PROJECT_VERSION_SHA)
#  set(PROJECT_VERSION_SHA v0.0.0-unknown)
#  set(PROJECT_VERSION_SHORT v0.0.0)
#  message(WARNING "Failed to determine PROJECT_VERSION_SHA from Git tags. Using default version \"${PROJECT_VERSION_SHA}\".")
#endif()

########### Create project ############
project(${PROJECT_NAME} VERSION ${PROJECT_VERSION})
message(STATUS "Project ${PROJECT_NAME} ${PROJECT_VERSION}")

########### Setup ############
# Set env variable
if (CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "Generate CMake cache for Release")
    if (WIN32)
        set(SUB_SYS WIN32)
    endif(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
else ()
    message(STATUS "Generate CMake cache for Debug")
    add_definitions(-DDEBUG)
endif (CMAKE_BUILD_TYPE MATCHES Release)

# Graphic API
option(USE_OPENGL_API "Use OpenGL graphic api" FALSE)

message(STATUS "USE_OPENGL_API: ${USE_OPENGL_API}")

########### Build ############
file(GLOB_RECURSE project_source_files LIST_DIRECTORIES false CONFIGURE_DEPENDS src/*.cpp src/*.c)

file(GLOB_RECURSE project_headers  LIST_DIRECTORIES false CONFIGURE_DEPENDS include/*.hpp include/*.h)
message(STATUS "Headers: ${project_headers}")
message(STATUS "Sources : ${project_source_files}")

# Set include directory
include_directories("${ABS_INCLUDE_DIR}/")

# Set icon
if (WIN32)
    set(APP_ICON_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")
endif ()

# add the executable
add_executable(${PROJECT_NAME} ${SUB_SYS} ${project_headers} ${project_source_files} ${APP_ICON_RESOURCE})

########### Install setup ############
# Setup install process
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR}/)

if (WIN32)
  install(DIRECTORY ${REL_RESOURCES_DIR} DESTINATION ${CMAKE_INSTALL_DATADIR}/)
else()
  install(DIRECTORY ${REL_RESOURCES_DIR} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR})
endif()

########### Deps ############
# GLFW
set (GLFW_BUILD_EXAMPLES OFF)
set (GLFW_BUILD_TESTS OFF)
set (GLFW_BUILD_DOCS OFF)
set (GLFW_INSTALL OFF)
add_subdirectory(${ABS_DEPS_DIR}/glfw)
target_link_libraries(${PROJECT_NAME} glfw)

# Graphic API
if(USE_OPENGL_API)
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
endif()

# Boxer
add_subdirectory(${ABS_DEPS_DIR}/Boxer)
target_link_libraries(${PROJECT_NAME} Boxer)

# yaml-cpp
add_subdirectory(${ABS_DEPS_DIR}/yaml-cpp)
target_link_libraries(${PROJECT_NAME} yaml-cpp)

# cpr
add_subdirectory(${ABS_DEPS_DIR}/cpr)
target_link_libraries(${PROJECT_NAME} cpr)

# imgui
option(IMGUI_IMPL_OPENGL3 ON)
set(IMGUI_EXAMPLES OFF)
set(IMGUI_DEMO OFF)
set(IMGUI_ENABLE_FREETYPE OFF)
set(IMGUI_IMPL_SDL OFF)
set(IMGUI_IMPL_METAL OFF)
set(IMGUI_IMPL_OSX OFF)
set(IMGUI_IMPL_WIN32 OFF)
set(IMGUI_IMPL_GLFW OFF)
set(IMGUI_IMPL_GLUT OFF)
set(IMGUI_IMPL_OPENGL2 OFF)
set(IMGUI_IMPL_DX9 OFF)
set(IMGUI_IMPL_DX10 OFF)
set(IMGUI_IMPL_DX11 OFF)
set(IMGUI_IMPL_DX12 OFF)
set(IMGUI_IMPL_VULKAN OFF)
add_subdirectory(${ABS_DEPS_DIR}/imgui)
target_link_libraries(${PROJECT_NAME} imgui)

########### Define ############
target_compile_definitions(${PROJECT_NAME} PRIVATE RESOURCE_PATH="${REL_RESOURCES_DIR}/")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_NAME="${PROJECT_NAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")

# Graphic API
if(USE_OPENGL_API)
    target_compile_definitions(${PROJECT_NAME} PRIVATE SHADER_RESOURCE_PATH="${REL_RESOURCES_DIR}/shader/glsl/")
    target_compile_definitions(${PROJECT_NAME} PRIVATE SHADER_VERTEX_EXT=".vs")
    target_compile_definitions(${PROJECT_NAME} PRIVATE SHADER_FRAG_EXT=".fs")
    target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_OPENGL_API)
endif()

########### Build link ############
# Set a symbolic link so exe can find the /Data folder
add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/${REL_RESOURCES_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${REL_RESOURCES_DIR}
)
add_dependencies(${PROJECT_NAME} copy_assets)