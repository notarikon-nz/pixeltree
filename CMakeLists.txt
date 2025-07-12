# PixelTree/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

# Project definition with version
project(PixelTree 
    VERSION 1.0.0
    DESCRIPTION "High-performance procedural 2D tree generator"
    LANGUAGES CXX
)

# C++20 requirement
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type default
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Project options
option(PIXELTREE_BUILD_TESTS "Build tests" ON)
option(PIXELTREE_BUILD_EXAMPLES "Build examples" ON)
option(PIXELTREE_BUILD_TOOLS "Build development tools" OFF)
option(PIXELTREE_HEADER_ONLY "Build as header-only library" OFF)
option(PIXELTREE_ENABLE_SIMD "Enable SIMD optimizations" ON)
option(PIXELTREE_ENABLE_OPENMP "Enable OpenMP parallelization" ON)
option(PIXELTREE_BUILD_SHARED "Build shared library" OFF)

# Include CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CompilerWarnings)

# Find dependencies
find_package(PNG QUIET)
if(PIXELTREE_ENABLE_OPENMP)
    find_package(OpenMP QUIET)
endif()

# Custom SIMD detection
if(PIXELTREE_ENABLE_SIMD)
    include(FindSIMD)
endif()

# vcpkg integration
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

# Main library target
if(PIXELTREE_HEADER_ONLY)
    add_library(speedtree2d INTERFACE)
    target_include_directories(speedtree2d INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
else()
    add_library(speedtree2d)
    target_include_directories(speedtree2d 
        PUBLIC 
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    
    # Add source files
    file(GLOB_RECURSE PIXELTREE_SOURCES 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
    )
    target_sources(speedtree2d PRIVATE ${PIXELTREE_SOURCES})
endif()

# Compiler-specific settings
set_project_warnings(speedtree2d)

# Feature configuration
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/speedtree2d/config.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/speedtree2d/config.hpp
    @ONLY
)

# Link dependencies
if(PNG_FOUND)
    target_link_libraries(speedtree2d PRIVATE PNG::PNG)
    target_compile_definitions(speedtree2d PRIVATE PIXELTREE_HAS_PNG)
endif()

if(OpenMP_FOUND AND PIXELTREE_ENABLE_OPENMP)
    target_link_libraries(speedtree2d PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(speedtree2d PRIVATE PIXELTREE_HAS_OPENMP)
endif()

# SIMD support
if(PIXELTREE_ENABLE_SIMD)
    if(SIMD_SSE2_FOUND)
        target_compile_definitions(speedtree2d PRIVATE PIXELTREE_HAS_SSE2)
    endif()
    if(SIMD_AVX2_FOUND)
        target_compile_definitions(speedtree2d PRIVATE PIXELTREE_HAS_AVX2)
    endif()
endif()

# Platform-specific settings
if(WIN32)
    target_compile_definitions(speedtree2d PRIVATE PIXELTREE_PLATFORM_WINDOWS)
elseif(UNIX)
    target_compile_definitions(speedtree2d PRIVATE PIXELTREE_PLATFORM_UNIX)
endif()

# Subdirectories
if(PIXELTREE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(PIXELTREE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(PIXELTREE_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

# Installation
install(TARGETS speedtree2d
    EXPORT PixelTreeTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/speedtree2d/config.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/speedtree2d
)

# Package configuration
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/PixelTreeConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PixelTreeConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/PixelTreeConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PixelTree
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/PixelTreeConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/PixelTreeConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PixelTree
)

install(EXPORT PixelTreeTargets
    FILE PixelTreeTargets.cmake
    NAMESPACE PixelTree::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PixelTree
)