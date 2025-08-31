cmake_minimum_required(VERSION 3.16)
project(Warfare_Observation_3d_Engagement C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(WOE_BUILD_EXAMPLES "Build examples" OFF)

# Dependencies: raylib via FetchContent
include(FetchContent)
# Share FetchContent downloads/builds em ~/.cache/cmake por padrão
if(NOT DEFINED FETCHCONTENT_BASE_DIR)
  set(FETCHCONTENT_BASE_DIR "$ENV{HOME}/.cache/cmake" CACHE PATH "Base dir for FetchContent downloads" FORCE)
endif()

set(RAYLIB_VERSION 5.0)
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG ${RAYLIB_VERSION}
)

# Prefer system packages if available
find_package(raylib ${RAYLIB_VERSION} QUIET)
if(NOT raylib_FOUND)
  message(STATUS "raylib not found system-wide; fetching ${RAYLIB_VERSION}...")
  FetchContent_MakeAvailable(raylib)
endif()

# Compile definitions and warnings
if (MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

# Source
add_executable(woe3d
  src/main.c
)

# On Linux we need to link extra libs that raylib expects sometimes
if(UNIX AND NOT APPLE)
  find_package(PkgConfig QUIET)
  if (PkgConfig_FOUND)
    pkg_check_modules(GLFW3 QUIET glfw3)
  endif()
endif()

# Link
target_link_libraries(woe3d PRIVATE raylib)

# Include dirs
if (TARGET raylib)
  target_include_directories(woe3d PRIVATE ${raylib_SOURCE_DIR}/src)
endif()

# Resources install (none for now)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Install
install(TARGETS woe3d RUNTIME DESTINATION bin)

# IDE folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# --- Documentation (Doxygen) ---
find_package(Doxygen QUIET)
if (DOXYGEN_FOUND)
  add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)
else()
  message(STATUS "Doxygen not found: target 'doc' will be unavailable")
endif()
