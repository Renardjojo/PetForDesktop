#
# CMake build system for Dear ImGui
# =================================
#
# Build instructions:
# 1. Install latest CMake
#   * Windows: https://cmake.org/download/ (Tick checkbox to place cmake in system PATH)
#   * Linux: from your favorite package manager
#   * MacOS: brew install cmake
# 2. Open command prompt in directory containing "imgui" and "imgui_dev" folders
#   * Windows: open Visual Studio tools command prompt or specify -G "Generator Name" (quotes are important).
#   * Other platforms: Specifying generator name is optional.
# 3. Create a build directory and configure build
#    git clone https://github.com/ocornut/imgui.git
#    mkdir cmake-build
#    cd cmake-build
#    cmake <optional build parameters> ../imgui
# 4. Build
#   * Windows (Visual Studio generators): Open generated imgui_dev.sln
#   * All platforms: cmake --build .
#
# Example of using Dear ImGui in your CMake project:
#
#   # Set various library options:
#   set(IMGUI_SDL_TARGET sdl CACHE STRING "" FORCE)
#   # Include Dear ImGui into build.
#   add_subdirectory(path/to/imgui)
#   <..>
#   # Link Dear ImGui to your application:
#   #  imgui - a main library target
#   #  imgui-sdl2 - a platform backend target
#   #  imgui-dx11 - a graphics backend target
#   target_link_libraries(YourApplication PUBLIC imgui imgui-sdl2 imgui-dx11)
#
# A list of defined targets:
#   imgui-sdl2: platform backend for Windows/Linux/MacOS/etc using SDL2.
#   imgui-glfw: platform backend for Windows/Linux/MacOS/etc using GLFW.
#   imgui-osx: platform backend for MacOS.
#   imgui-win32: platform backend for Windows.
#   imgui-glut: platform backend for Windows/Linux/MacOS using glut/freeglut with OpenGL.
#   imgui-metal: graphics backend for MacOS using Metal graphics API.
#   imgui-opengl3: graphics backend for Windows/Linux/MacOS/etc using OpenGL3 graphics API.
#   imgui-opengl2: graphics backend for Windows/Linux/MacOS/etc using OpenGL2 graphics API.
#   imgui-vulkan: graphics backend for Windows/Linux/MacOS/etc using Vulkan graphics API.
#   imgui-dx9: graphics backend for Windows using DirectX 9 graphics API.
#   imgui-dx10: graphics backend for Windows using DirectX 10 graphics API.
#   imgui-dx11: graphics backend for Windows using DirectX 11 graphics API.
#   imgui-dx12: graphics backend for Windows using DirectX 12 graphics API.
#   imgui-sdlrenderer: graphics backend for platforms supported by SDL.
#
cmake_minimum_required (VERSION 3.13)
project (imgui)

# CMake: 3.13: option() honors normal variables. Sufficiently new CMake and this policy allow user to configure library
# by setting plain variables as opposed to cache variables as demonstrated in example.
cmake_policy(SET CMP0077 NEW)

###############################################################################
# Supported build parameters
###############################################################################
option(IMGUI_EXAMPLES "Build ImGui examples" ON)
option(IMGUI_DEMO "Include the ImGui demo window implementation in library" ON)

option(IMGUI_DISABLE_OBSOLETE_FUNCTIONS "Disable deprecated functions" ON)
option(IMGUI_DISABLE_OBSOLETE_KEYIO "Disable legacy input handling" ON)
option(IMGUI_DISABLE_FILE_FUNCTIONS "Disable use of file functions" OFF)
option(IMGUI_ENABLE_STDLIB_SUPPORT "Enable extra functions taking stdlib types" OFF)
option(IMGUI_ENABLE_COVERAGE "Enable coverage testing for supported compilers" OFF)
option(IMGUI_ENABLE_ASAN "Enable compiler-based sanitizers" OFF)
option(IMGUI_ENABLE_TRACY "Enable tracy profiling" OFF)
option(IMGUI_ENABLE_FREETYPE "Enable FreeType font rasterizer" ON)
option(IMGUI_IMPL_SDL "Build the SDL implementation (only if supported)" ON)
option(IMGUI_IMPL_METAL "Build the Metal implementation (only if supported)" ${APPLE})
option(IMGUI_IMPL_OSX "Build the OSX implementation (only if supported)" ${APPLE})
option(IMGUI_IMPL_WIN32 "Build the Win32 (native winapi) implementation (only if supported)" ${WIN32})
option(IMGUI_IMPL_GLFW "Build the GLFW implementation (only if supported)" ON)
option(IMGUI_IMPL_GLUT "Build the GLUT implementation (only if supported)" ON)
option(IMGUI_IMPL_OPENGL3 "Build the OpenGL 3 implementation (only if supported)" ON)
option(IMGUI_IMPL_OPENGL2 "Build the OpenGL 2 (legacy) implementation (only if supported)" ${IMGUI_IMPL_OPENGL3})
option(IMGUI_IMPL_DX9 "Build the DirectX 9 implementation (only if supported)" ${WIN32})
option(IMGUI_IMPL_DX10 "Build the DirectX 10 implementation (only if supported)" ${WIN32})
option(IMGUI_IMPL_DX11 "Build the DirectX 11 implementation (only if supported)" ${WIN32})
option(IMGUI_IMPL_DX12 "Build the DirectX 12 implementation (only if supported)" ${WIN32})
option(IMGUI_IMPL_VULKAN "Build the Vulkan implementation (only if supported)" ON)

set(IMGUI_SDL_TARGET "" CACHE STRING "A custom SDL target name that will be used for linking to examples.")
set(IMGUI_GLFW_TARGET "" CACHE STRING "A custom GLFW target name that will be used for linking to examples.")
set(IMGUI_GLUT_TARGET "" CACHE STRING "A custom GLFW target name that will be used for linking to examples.")

###############################################################################

if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set (IMGUI_IMPL_GLFW OFF)
    set (IMGUI_IMPL_GLUT OFF)
    set (CMAKE_POSITION_INDEPENDENT_CODE ON)
endif ()

# Tidy up build dir, but only if user has not configured it yet.
if (NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif ()
if (NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif ()
if (NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif ()

# Minimal supported C++ standard.
if (NOT DEFINED CMAKE_CXX_STANDARD)
    set (CMAKE_CXX_STANDARD 11)
endif ()

# Helper variables. They allow to use this CMakeLists.txt file while it is in examples or in root directory. You may
# copy/symlink this file to imgui root directory in order to help IDEs like CLion to recognize headers as part of the
# project.
if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/imgui.h)
    # CMakeLists.txt is in root directory.
    set (IMGUI_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
    set (IMGUI_EXAMPLES_DIR ${IMGUI_ROOT_DIR}/examples)
    set (IMGUI_BACKENDS_DIR ${IMGUI_ROOT_DIR}/backends)
    set (IMGUI_MISC_DIR ${IMGUI_ROOT_DIR}/misc)
elseif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui.h)
    # CMakeLists.txt is in the examples directory.
    set (IMGUI_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/imgui)
    set (IMGUI_EXAMPLES_DIR ${IMGUI_ROOT_DIR}/imgui/examples)
    set (IMGUI_BACKENDS_DIR ${IMGUI_ROOT_DIR}/imgui/backends)
    set (IMGUI_MISC_DIR ${IMGUI_ROOT_DIR}/imgui/misc)
else ()
    # CMakeLists.txt is in examples directory.
    set (IMGUI_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
    set (IMGUI_EXAMPLES_DIR ${CMAKE_CURRENT_LIST_DIR})
    set (IMGUI_BACKENDS_DIR ${CMAKE_CURRENT_LIST_DIR}/../backends)
    set (IMGUI_MISC_DIR ${IMGUI_ROOT_DIR}/../misc)
endif ()

###############################################################################
# Warnings
###############################################################################
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wunused-parameter -Wformat -Wnarrowing -Wno-sign-conversion -Wno-error=declaration-after-statement) # $<$<COMPILE_LANGUAGE:CXX>:-pedantic>
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-Wnontrivial-memaccess)
    endif ()
    if (NOT APPLE)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-copy>)
    endif ()
endif ()

# These warnings are overzealous in GCC and only practical to use with Clang.
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wshadow)
endif ()

# Disable warnings for third party source code. We can disable warnings for tracy, because it's source code is built as
# a part of our own build system. In case SDL2/GLFW/FreeType are used from a source distribution, we can not modify
# build flags of those targets from over here and thus they will have a warning spam.
file (GLOB_RECURSE THIRD_PARTY_SOURCE_FILES ${IMGUI_EXAMPLES_DIR}/libs/* imstb_*.c ../tracy/*.cpp)
set_source_files_properties(${THIRD_PARTY_SOURCE_FILES} PROPERTIES COMPILE_OPTIONS -w)

#if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
#    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-class-memaccess>)
#endif ()

###############################################################################
# Main library
###############################################################################
# Set up main library. It is created as an INTERFACE library because it allows using different
# IMGUI_USER_CONFIG defines in different targets of downstream projects.
file(GLOB IMGUI_SOURCE_FILES ${IMGUI_ROOT_DIR}/*.h ${IMGUI_ROOT_DIR}/*.cpp)
if (NOT IMGUI_DEMO)
    list(REMOVE_ITEM IMGUI_SOURCE_FILES ${IMGUI_ROOT_DIR}/imgui_demo.cpp)
endif ()
add_library(imgui INTERFACE)
target_link_libraries(imgui INTERFACE ${CMAKE_DL_LIBS})
target_sources(imgui INTERFACE ${IMGUI_SOURCE_FILES})
if (IMGUI_DISABLE_OBSOLETE_FUNCTIONS)
    target_compile_definitions(imgui INTERFACE -DIMGUI_DISABLE_OBSOLETE_FUNCTIONS=1)
endif ()
if (IMGUI_DISABLE_OBSOLETE_KEYIO)
    target_compile_definitions(imgui INTERFACE -DIMGUI_DISABLE_OBSOLETE_KEYIO=1)
endif ()
if (IMGUI_DISABLE_FILE_FUNCTIONS)
    target_compile_definitions(imgui INTERFACE -DIMGUI_DISABLE_FILE_FUNCTIONS=1)
endif ()
if (IMGUI_USER_CONFIG)
    target_compile_definitions(imgui INTERFACE "-DIMGUI_USER_CONFIG=<${IMGUI_USER_CONFIG}>")
endif ()
target_include_directories(imgui INTERFACE ${IMGUI_ROOT_DIR})
if (NOT IMGUI_DEMO)
    target_compile_definitions(imgui INTERFACE -DIMGUI_DISABLE_DEMO_WINDOWS=1)
endif ()
if (IMGUI_ENABLE_STDLIB_SUPPORT)
    target_include_directories(imgui INTERFACE misc/cpp)
    target_sources(imgui INTERFACE misc/cpp/imgui_stdlib.h misc/cpp/imgui_stdlib.cpp)
endif ()
if (IMGUI_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(imgui INTERFACE -fprofile-arcs -ftest-coverage)
    target_link_libraries(imgui INTERFACE -fprofile-arcs -ftest-coverage)
    target_link_libraries(imgui INTERFACE gcov)
endif ()

if (IMGUI_ENABLE_ASAN AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(imgui INTERFACE -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls)
    target_link_libraries(imgui INTERFACE -g -fsanitize=address)
endif ()

###############################################################################
# Freetype
###############################################################################
if (IMGUI_ENABLE_FREETYPE)
    add_library(imgui-freetype INTERFACE)
    target_sources(imgui-freetype INTERFACE
        ${IMGUI_MISC_DIR}/freetype/imgui_freetype.h
        ${IMGUI_MISC_DIR}/freetype/imgui_freetype.cpp
    )

    find_package(Freetype)
    if (FREETYPE_FOUND)
        target_include_directories(imgui-freetype INTERFACE ${FREETYPE_INCLUDE_DIRS})
        target_link_libraries(imgui-freetype INTERFACE ${FREETYPE_LIBRARIES})
    elseif (EXISTS ../freetype)
        set (SKIP_INSTALL_ALL ON)
        add_subdirectory(../freetype ${CMAKE_BINARY_DIR}/freetype)
        target_link_libraries(imgui-freetype INTERFACE freetype)
    else ()
        message(FATAL_ERROR "IMGUI_ENABLE_FREETYPE but FreeType was not found.")
    endif ()
    target_compile_definitions(imgui-freetype INTERFACE -DIMGUI_ENABLE_FREETYPE=1)
    target_link_libraries(imgui INTERFACE imgui-freetype)
endif()

###############################################################################
# C++ standard library
###############################################################################
add_library(imgui-stdlib INTERFACE)
target_sources(imgui-stdlib INTERFACE
    ${IMGUI_MISC_DIR}/cpp/imgui_stdlib.h
    ${IMGUI_MISC_DIR}/cpp/imgui_stdlib.cpp
)

###############################################################################
# Tracy
###############################################################################
if (IMGUI_ENABLE_TRACY AND EXISTS ${IMGUI_ROOT_DIR}/../tracy)
    add_library(TracyLib INTERFACE)
    target_sources(TracyLib INTERFACE ../tracy/public/TracyClient.cpp)
    find_package(Threads REQUIRED)
    target_link_libraries(TracyLib INTERFACE Threads::Threads ${CMAKE_DL_LIBS})
    target_compile_definitions(TracyLib INTERFACE -DTRACY_ENABLE=1) # -DTRACY_VERBOSE=1
    target_include_directories(TracyLib INTERFACE ../tracy/public)
    target_link_libraries(imgui INTERFACE TracyLib)
endif ()

###############################################################################
# Backends
###############################################################################
# Ignore silently when required headers are missing.
set(CMAKE_REQUIRED_QUIET ON)
include(CheckIncludeFile)

# PkgConfig will be needed for SDL and glfw on some platforms.
find_package(PkgConfig QUIET)

if (IMGUI_IMPL_SDL)
    find_package(SDL2 QUIET)
    if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
        # Nothing to do for emscripten builds. SDL is bunled with the compiler.
    elseif (IMGUI_SDL_TARGET)
        # Custom user target was set. No auto-detection required.
    elseif (EXISTS ${IMGUI_ROOT_DIR}/../SDL2/CMakeLists.txt)
        # When parent directory contains SDL2 source code - we can build it directly.
        add_subdirectory (${IMGUI_ROOT_DIR}/../SDL2 ${CMAKE_CURRENT_BINARY_DIR}/SDL2)
        set(IMGUI_SDL_TARGET SDL2-static)
    elseif (TARGET SDL2::SDL2)
        # Some platforms (Linux) have SDL target properly exported as CMake target.
    elseif (SDL2_FOUND)
        # Platforms that do not export target but use old CMake conventions can be handled this way.
        add_library(SDL2::SDL2 INTERFACE IMPORTED)
        target_link_libraries(SDL2::SDL2 INTERFACE ${SDL2_LIBRARIES})
        target_include_directories(SDL2::SDL2 INTERFACE ${SDL2_INCLUDE_DIRS})
    elseif (NOT "$ENV{SDL2_DIR}" STREQUAL "")
        # On windows we may set SDL2_DIR environment variable and link to binary SDL distribution.
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(IMGUI_SDL_PLATFORM_ARCH x64)
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(IMGUI_SDL_PLATFORM_ARCH x86)
        else ()
            message(FATAL_ERROR "Unsupported platform.")
        endif()
        add_library(SDL2::SDL2 SHARED IMPORTED)
        target_include_directories(SDL2::SDL2 INTERFACE "$ENV{SDL2_DIR}/include")
        target_link_libraries(SDL2::SDL2 INTERFACE $ENV{SDL2_DIR}/lib/${IMGUI_SDL_PLATFORM_ARCH}/SDL2main.lib)
        set_target_properties(SDL2::SDL2 PROPERTIES
            IMPORTED_LOCATION "$ENV{SDL2_DIR}/lib/${IMGUI_SDL_PLATFORM_ARCH}/SDL2.dll"
            IMPORTED_IMPLIB "$ENV{SDL2_DIR}/lib/${IMGUI_SDL_PLATFORM_ARCH}/SDL2.lib"
        )
    elseif (PkgConfig_FOUND)
        # Rest of the platforms (like MacOS) can consume SDL via pkg-config.
        # CMake 3.6 supports IMPORTED_TARGET parameter which creates PkgConfig::sdl2 target that we can easily link to.
        # TODO: Switch to using IMPORTED_TARGET when CMake minimal version is increased.
        pkg_check_modules(SDL2 QUIET sdl2)
        if (SDL2_FOUND)
            add_library(SDL2::SDL2 INTERFACE IMPORTED)
            target_link_libraries(SDL2::SDL2 INTERFACE ${SDL2_LDFLAGS})
            target_compile_options(SDL2::SDL2 INTERFACE ${SDL2_CFLAGS})
        endif ()
    endif ()
    if (NOT IMGUI_SDL_TARGET)
        set(IMGUI_SDL_TARGET SDL2::SDL2)
    endif ()
    if (TARGET ${IMGUI_SDL_TARGET} OR CMAKE_SYSTEM_NAME STREQUAL Emscripten)
        # SDL platform
        add_library (imgui-sdl2 INTERFACE)
        target_sources(imgui-sdl2 INTERFACE
            ${IMGUI_BACKENDS_DIR}/imgui_impl_sdl2.h
            ${IMGUI_BACKENDS_DIR}/imgui_impl_sdl2.cpp
        )
        target_include_directories(imgui-sdl2 SYSTEM INTERFACE ${IMGUI_BACKENDS_DIR})
        target_link_libraries(imgui-sdl2 INTERFACE imgui)
        if (TARGET ${IMGUI_SDL_TARGET})
            target_link_libraries(imgui-sdl2 INTERFACE ${IMGUI_SDL_TARGET})
        endif ()

        # SDL renderer
        add_library (imgui-sdlrenderer INTERFACE)
        target_sources(imgui-sdlrenderer INTERFACE
            ${IMGUI_BACKENDS_DIR}/imgui_impl_sdlrenderer.h
            ${IMGUI_BACKENDS_DIR}/imgui_impl_sdlrenderer.cpp
        )
        target_include_directories(imgui-sdlrenderer SYSTEM INTERFACE ${IMGUI_BACKENDS_DIR})
        target_link_libraries(imgui-sdlrenderer INTERFACE imgui)
        if (TARGET ${IMGUI_SDL_TARGET})
            target_link_libraries(imgui-sdlrenderer INTERFACE ${IMGUI_SDL_TARGET})
        endif ()
    else ()
        message(STATUS "IMGUI_IMPL_SDL set to ON but SDL2 could not be found.")
    endif ()
endif ()
if (IMGUI_IMPL_METAL AND APPLE)
    add_library(imgui-metal INTERFACE)
    target_sources(imgui-metal INTERFACE
        ${IMGUI_BACKENDS_DIR}/imgui_impl_metal.h
        ${IMGUI_BACKENDS_DIR}/imgui_impl_metal.mm
    )
    target_link_libraries(imgui-metal INTERFACE imgui "-framework Cocoa" "-framework Metal" "-framework QuartzCore")
    target_compile_options(imgui-metal INTERFACE -fobjc-arc)
    target_include_directories(imgui-metal SYSTEM INTERFACE ${IMGUI_BACKENDS_DIR})
endif ()
if (IMGUI_IMPL_OSX AND APPLE)
    add_library(imgui-osx INTERFACE)
    target_sources(imgui-osx INTERFACE
        ${IMGUI_BACKENDS_DIR}/imgui_impl_osx.h
        ${IMGUI_BACKENDS_DIR}/imgui_impl_osx.mm
    )
    target_link_libraries(imgui-osx INTERFACE imgui "-framework Cocoa" "-framework AppKit")
    target_compile_options(imgui-osx INTERFACE -fobjc-arc)
    target_include_directories(imgui-osx SYSTEM INTERFACE ${IMGUI_BACKENDS_DIR})
endif ()
if (IMGUI_IMPL_WIN32 AND WIN32)
    add_library(imgui-win32 INTERFACE)
    target_sources(imgui-win32 INTERFACE
        ${IMGUI_BACKENDS_DIR}/imgui_impl_win32.h
        ${IMGUI_BACKENDS_DIR}/imgui_impl_win32.cpp
    )
    target_include_directories(imgui-win32 SYSTEM INTERFACE ${IMGUI_BACKENDS_DIR})
endif ()
if (IMGUI_IMPL_GLFW)
    if (IMGUI_GLFW_TARGET)
        # Custom user target was set. No auto-detection required.
    elseif (TARGET glfw)
        # GLFW exists already. Nothing to do.
    elseif (EXISTS ${IMGUI_ROOT_DIR}/../glfw/CMakeLists.txt)
        # When parent directory contains GLFW source code - we can build it directly.
        set (GLFW_STANDALONE OFF)
        set (GLFW_INSTALL OFF)
        set (GLFW_BUILD_DOCS OFF)
        add_subdirectory (${IMGUI_ROOT_DIR}/../glfw ${CMAKE_CURRENT_BINARY_DIR}/glfw)
    elseif (MSVC)
        # We ship compiled libraries in our repository for Visual Studio.
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(IMGUI_GLFW_PLATFORM_BITS 64)
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(IMGUI_GLFW_PLATFORM_BITS 32)
        else ()
            message(FATAL_ERROR "Unsupported platform.")
        endif()
        add_library(glfw INTERFACE IMPORTED)
        target_link_libraries(glfw INTERFACE ${IMGUI_ROOT_DIR}/examples/libs/glfw/lib-vc2010-${IMGUI_GLFW_PLATFORM_BITS}/glfw3.lib)
        target_include_directories(glfw INTERFACE ${IMGUI_ROOT_DIR}/examples/libs/glfw/include)
    elseif (PkgConfig_FOUND)
        # CMake 3.6 supports IMPORTED_TARGET parameter which creates PkgConfig::glfw target that we can easily link to.
        # TODO: Switch to using IMPORTED_TARGET when CMake minimal version is increased.
        pkg_check_modules(GLFW QUIET glfw3)
        if (GLFW_FOUND)
            add_library(glfw INTERFACE IMPORTED)
            target_link_libraries(glfw INTERFACE ${GLFW_LDFLAGS})
            target_compile_options(glfw INTERFACE ${GLFW_CFLAGS})
        endif ()
    endif ()
    if (NOT IMGUI_GLFW_TARGET)
        set(IMGUI_GLFW_TARGET glfw)
    endif ()
    if (TARGET ${IMGUI_GLFW_TARGET})
        add_library(imgui-glfw INTERFACE)
        target_sources(imgui-glfw INTERFACE
            ${IMGUI_BACKENDS_DIR}/imgui_impl_glfw.h
            ${IMGUI_BACKENDS_DIR}/imgui_impl_glfw.cpp
        )
        target_link_libraries(imgui-glfw INTERFACE ${CMAKE_DL_LIBS} ${IMGUI_GLFW_TARGET})
        target_include_directories(imgui-glfw SYSTEM INTERFACE ${IMGUI_BACKENDS_DIR})
    else ()
        message(STATUS "IMGUI_IMPL_GLFW set to ON but GLFW could not be found.")
    endif ()
endif ()
if (IMGUI_IMPL_OPENGL2 OR IMGUI_IMPL_OPENGL3)
    set(OpenGL_GL_PREFERENCE "GLVND")   # GLVND is vendor-agnostic OpenGL dispatch library on Linux.
    include(FindOpenGL)
    if (NOT OPENGL_FOUND)
        message(FATAL_ERROR "IMGUI_IMPL_OPENGL2 or IMGUI_IMPL_OPENGL3 set to ON but OpenGL could not be found.")
    endif ()
    if (NOT TARGET OpenGL::GL)
        add_library(OpenGL::GL INTERFACE IMPORTED)
        target_include_directories(OpenGL::GL INTERFACE ${OPENGL_INCLUDE_DIR})
        target_link_libraries(OpenGL::GL INTERFACE ${OPENGL_LIBRARY})
    endif ()
    if (IMGUI_IMPL_OPENGL2)
        add_library(imgui-opengl2 INTERFACE)
        target_sources(imgui-opengl2 INTERFACE ${IMGUI_BACKENDS_DIR}/imgui_impl_opengl2.cpp)
        target_link_libraries (imgui-opengl2 INTERFACE imgui OpenGL::GL)
        target_include_directories(imgui-opengl2 SYSTEM INTERFACE ${IMGUI_BACKENDS_DIR})
    endif ()
    if (IMGUI_IMPL_OPENGL3)
        add_library(imgui-opengl3 INTERFACE)
        target_sources(imgui-opengl3 INTERFACE ${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.cpp)
        target_link_libraries (imgui-opengl3 INTERFACE imgui OpenGL::GL)
        target_include_directories(imgui-opengl3 SYSTEM INTERFACE ${IMGUI_BACKENDS_DIR})
    endif ()
endif ()
if (IMGUI_IMPL_GLUT)
    find_package(GLUT QUIET)
    if (GLUT_glut_LIBRARY OR IMGUI_GLUT_TARGET)
        add_library(imgui-glut INTERFACE)
        target_sources(imgui-glut INTERFACE
            ${IMGUI_BACKENDS_DIR}/imgui_impl_glut.h
            ${IMGUI_BACKENDS_DIR}/imgui_impl_glut.cpp
        )
        target_link_libraries(imgui-glut INTERFACE imgui)
        if (IMGUI_GLUT_TARGET)
            target_link_libraries(imgui-glut INTERFACE ${IMGUI_GLUT_TARGET})
        else ()
            target_include_directories(imgui-glut SYSTEM INTERFACE ${GLUT_INCLUDE_DIR} ${IMGUI_BACKENDS_DIR})
            target_link_libraries(imgui-glut INTERFACE imgui ${GLUT_glut_LIBRARY})
            if (APPLE)
                target_link_libraries(imgui-glut INTERFACE ${GLUT_cocoa_LIBRARY})
            endif ()
        endif ()
    else ()
        message (STATUS "IMGUI_IMPL_GLUT set to ON but GLUT could not be found.")
    endif ()
endif ()
if (IMGUI_IMPL_DX9)
    check_include_file(d3d9.h HAS_D3D9)
    if (HAS_D3D9)
        add_library(imgui-dx9 INTERFACE)
        target_sources(imgui-dx9 INTERFACE
            ${IMGUI_BACKENDS_DIR}/imgui_impl_dx9.h
            ${IMGUI_BACKENDS_DIR}/imgui_impl_dx9.cpp
        )
        target_link_libraries(imgui-dx9 INTERFACE imgui d3d9)
        target_include_directories(imgui-dx9 SYSTEM INTERFACE ${IMGUI_BACKENDS_DIR})
    else ()
        message (STATUS "IMGUI_IMPL_DX9 set to ON but DirectX 9 could not be found.")
    endif ()
endif ()
if (IMGUI_IMPL_DX10)
    check_include_file(d3d10.h HAS_D3D10)
    if (HAS_D3D10)
        add_library(imgui-dx10 INTERFACE)
        target_sources(imgui-dx10 INTERFACE
            ${IMGUI_BACKENDS_DIR}/imgui_impl_dx10.h
            ${IMGUI_BACKENDS_DIR}/imgui_impl_dx10.cpp
        )
        target_link_libraries(imgui-dx10 INTERFACE imgui d3d10)
        target_include_directories(imgui-dx10 SYSTEM INTERFACE ${IMGUI_BACKENDS_DIR})
    else ()
        message (STATUS "IMGUI_IMPL_DX10 set to ON but DirectX 10 could not be found.")
    endif ()
endif ()
if (IMGUI_IMPL_DX11)
    check_include_file(d3d11.h HAS_D3D11)
    if (HAS_D3D11)
        add_library(imgui-dx11 INTERFACE)
        target_sources(imgui-dx11 INTERFACE
            ${IMGUI_BACKENDS_DIR}/imgui_impl_dx11.h
            ${IMGUI_BACKENDS_DIR}/imgui_impl_dx11.cpp
        )
        target_link_libraries(imgui-dx11 INTERFACE imgui d3d11)
        target_include_directories(imgui-dx11 SYSTEM INTERFACE ${IMGUI_BACKENDS_DIR})
    else ()
        message (STATUS "IMGUI_IMPL_DX11 set to ON but DirectX 11 could not be found.")
    endif ()
endif ()
if (IMGUI_IMPL_DX12)
    check_include_file(d3d12.h HAS_D3D12)
    if (HAS_D3D11)
        add_library(imgui-dx12 INTERFACE)
        target_sources(imgui-dx12 INTERFACE
            ${IMGUI_BACKENDS_DIR}/imgui_impl_dx12.h
            ${IMGUI_BACKENDS_DIR}/imgui_impl_dx12.cpp
        )
        target_link_libraries(imgui-dx12 INTERFACE imgui d3d12 dxgi)
        target_include_directories(imgui-dx12 SYSTEM INTERFACE ${IMGUI_BACKENDS_DIR})
    else ()
        message (STATUS "IMGUI_IMPL_DX12 set to ON but DirectX 12 could not be found.")
    endif ()
endif ()
if (IMGUI_IMPL_VULKAN)
    find_package(Vulkan QUIET)
    if (CMAKE_CXX_STANDARD EQUAL 98)
        message(STATUS "IMGUI_IMPL_VULKAN set to ON but Vulkan SDK requires at least C++11.")
    elseif (NOT Vulkan_FOUND)
        message(STATUS "IMGUI_IMPL_VULKAN set to ON but Vulkan could not be found.")
    else ()
        add_library(imgui-vulkan INTERFACE)
        target_sources(imgui-vulkan INTERFACE ${IMGUI_BACKENDS_DIR}/imgui_impl_vulkan.cpp)
        target_link_libraries (imgui-vulkan INTERFACE imgui ${Vulkan_LIBRARIES} OpenGL::GL)
        target_include_directories(imgui-vulkan SYSTEM INTERFACE ${Vulkan_INCLUDE_DIRS} ${IMGUI_BACKENDS_DIR})
    endif ()
endif ()

###############################################################################
# Examples
###############################################################################
if (IMGUI_EXAMPLES)
    function (IMGUI_EXAMPLE)
        cmake_parse_arguments(IMGUI_EXAMPLE "" "TARGET;SOURCE" "LINKS" ${ARGN})
        if (NOT IMGUI_EXAMPLE_SOURCE)
            set (IMGUI_EXAMPLE_SOURCE main.cpp)
        endif ()
        # Check whether dependency libraries are available.
        foreach (dep ${IMGUI_EXAMPLE_LINKS})
            if (NOT TARGET ${dep})
                return ()
            endif ()
        endforeach ()
        # Resize a target.
        add_executable(${IMGUI_EXAMPLE_TARGET} ${IMGUI_EXAMPLES_DIR}/${IMGUI_EXAMPLE_TARGET}/${IMGUI_EXAMPLE_SOURCE})
        target_link_libraries(${IMGUI_EXAMPLE_TARGET} PRIVATE imgui ${IMGUI_EXAMPLE_LINKS})
        list(FIND IMGUI_EXAMPLE_LINKS imgui-sdl2 LINKS_SDL)
        if (NOT LINKS_SDL EQUAL -1 AND WIN32)
            get_target_property(SDL_IS_IMPORTED ${IMGUI_SDL_TARGET} IMPORTED)
            if (SDL_IS_IMPORTED)
                # When program links to imgui-sdl2 and imported target ${IMGUI_SDL_TARGET} exists - copy SDL dll.
                add_custom_command(TARGET ${IMGUI_EXAMPLE_TARGET} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            $<TARGET_FILE:${IMGUI_SDL_TARGET}> $<TARGET_FILE_DIR:${IMGUI_EXAMPLE_TARGET}>)
            endif ()
        endif ()
    endfunction()
    IMGUI_EXAMPLE(TARGET example_null LINKS imgui-null)
    if (APPLE)
        # MacOS Metal example
        # FIXME: This sample is still broken.
        add_executable(example_apple_metal MACOSX_BUNDLE
            ${IMGUI_EXAMPLES_DIR}/example_apple_metal/Shared/main.m
            ${IMGUI_EXAMPLES_DIR}/example_apple_metal/Shared/AppDelegate.h
            ${IMGUI_EXAMPLES_DIR}/example_apple_metal/Shared/AppDelegate.m
            ${IMGUI_EXAMPLES_DIR}/example_apple_metal/Shared/Renderer.h
            ${IMGUI_EXAMPLES_DIR}/example_apple_metal/Shared/Renderer.mm
            ${IMGUI_EXAMPLES_DIR}/example_apple_metal/Shared/ViewController.h
            ${IMGUI_EXAMPLES_DIR}/example_apple_metal/Shared/ViewController.mm
            ${IMGUI_EXAMPLES_DIR}/example_apple_metal/macOS/Base.lproj/Main.storyboard
        )
        target_link_libraries(example_apple_metal imgui imgui-osx imgui-metal)
        # Generate new plist
        file(READ ${IMGUI_EXAMPLES_DIR}/example_apple_metal/macOS/Info-macOS.plist INFO_PLIST)
        string(REPLACE "$(EXECUTABLE_NAME)" example_apple_metal INFO_PLIST "${INFO_PLIST}")
        string(REPLACE "$(PRODUCT_BUNDLE_IDENTIFIER)" org.imgui.example.apple-metal-macos INFO_PLIST "${INFO_PLIST}")
        string(REPLACE "$(MACOSX_DEPLOYMENT_TARGET)" 10.12 INFO_PLIST "${INFO_PLIST}")
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/example_apple_metal-info.plist "${INFO_PLIST}")
        # Consume new plist
        set_property(TARGET example_apple_metal PROPERTY
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/example_apple_metal-info.plist)
        # Compile storyboard
        if(NOT "${CMAKE_GENERATOR}" MATCHES "^Xcode.*")
            # Xcode compiles storyboard automatically. We must do it manually when using other build systems.
            find_program(IBTOOL ibtool HINTS /usr/bin ${OSX_DEVELOPER_ROOT}/usr/bin)
            add_custom_command(TARGET example_apple_metal POST_BUILD COMMAND ${IBTOOL} --errors --warnings --notices
                --output-format human-readable-text --compile
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/example_apple_metal.app/Contents/Resources/Base.lproj/Main.storyboardc
                ${IMGUI_EXAMPLES_DIR}/example_apple_metal/macOS/Base.lproj/Main.storyboard
                COMMENT "Compiling storyboard"
            )
        endif()
        IMGUI_EXAMPLE(TARGET example_apple_opengl2 LINKS imgui-osx imgui-opengl2 SOURCE main.mm)
        IMGUI_EXAMPLE(TARGET example_glfw_metal LINKS imgui-glfw imgui-metal SOURCE main.mm)
        IMGUI_EXAMPLE(TARGET example_sdl2_metal LINKS imgui-sdl2 imgui-metal SOURCE main.mm)
    endif ()
    if (WIN32)
        IMGUI_EXAMPLE(TARGET example_sdl2_directx11 LINKS imgui-sdl2 imgui-dx11)
        IMGUI_EXAMPLE(TARGET example_win32_directx9 LINKS imgui-win32 imgui-dx9)
        IMGUI_EXAMPLE(TARGET example_win32_directx10 LINKS imgui-win32 imgui-dx10)
        IMGUI_EXAMPLE(TARGET example_win32_directx11 LINKS imgui-win32 imgui-dx11)
        IMGUI_EXAMPLE(TARGET example_win32_directx12 LINKS imgui-win32 imgui-dx12)
    endif ()
    if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
        IMGUI_EXAMPLE(TARGET example_emscripten_opengl3 LINKS imgui-sdl2 imgui-opengl3)
        set_target_properties(example_emscripten_opengl3 PROPERTIES SUFFIX .html)
        target_compile_options(example_emscripten_opengl3 PUBLIC -sUSE_SDL=2)
        target_link_options(example_emscripten_opengl3 PUBLIC -sWASM=1 -sALLOW_MEMORY_GROWTH=1 -sDISABLE_EXCEPTION_CATCHING=1 -sNO_EXIT_RUNTIME=0)
        target_link_options(example_emscripten_opengl3 PUBLIC -sASSERTIONS=1 -sMAIN_MODULE=1)
        target_link_options(example_emscripten_opengl3 PUBLIC "--shell-file=${IMGUI_EXAMPLES_DIR}/example_emscripten_opengl3/shell_minimal.html")
        if (IMGUI_DISABLE_FILE_FUNCTIONS)
            target_link_options(example_emscripten_opengl3 PUBLIC -sNO_FILESYSTEM=1)
        else ()
            target_link_options(example_emscripten_opengl3 PUBLIC --no-heap-copy "--preload-file=${IMGUI_ROOT_DIR}/misc/fonts@/fonts")
        endif ()
        # Uncomment next line to fix possible rendering bugs with Emscripten version older then 1.39.0 (https://github.com/ocornut/imgui/issues/2877)
        #target_link_options(example_emscripten_opengl3 PUBLIC -sBINARYEN_TRAP_MODE=clamp)
        #target_link_options(example_emscripten_opengl3 PUBLIC -sSAFE_HEAP=1)    ## Adds overhead
        # TODO: Add example_emscripten_wgpu example
    else ()
        IMGUI_EXAMPLE(TARGET example_glfw_opengl2 LINKS imgui-glfw imgui-opengl2)
        IMGUI_EXAMPLE(TARGET example_glfw_opengl3 LINKS imgui-glfw imgui-opengl3)
        IMGUI_EXAMPLE(TARGET example_glfw_vulkan LINKS imgui-glfw imgui-vulkan)
        IMGUI_EXAMPLE(TARGET example_glut_opengl2 LINKS imgui-glut imgui-opengl2)
        IMGUI_EXAMPLE(TARGET example_sdl2_opengl2 LINKS imgui-sdl2 imgui-opengl2)
        IMGUI_EXAMPLE(TARGET example_sdl2_opengl3 LINKS imgui-sdl2 imgui-opengl3)
        IMGUI_EXAMPLE(TARGET example_sdl2_vulkan LINKS imgui-sdl2 imgui-vulkan)
        IMGUI_EXAMPLE(TARGET example_sdl2_sdlrenderer LINKS imgui-sdl2 imgui-sdlrenderer)
    endif ()
endif ()
