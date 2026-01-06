cmake_minimum_required(VERSION 3.10)
project(heic_converter VERSION 1.1.0 LANGUAGES CXX)  # CHANGED: 1.0.0 to 1.1.0

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configuration
if(DEFINED DEBIAN9_BUILD)
    add_definitions(-DDEBIAN9_BUILD)
    set(BUILD_TYPE "debian9")
    message(STATUS "Building for Debian 9")
elseif(DEFINED DEBIAN12_BUILD)
    add_definitions(-DDEBIAN12_BUILD)
    set(BUILD_TYPE "debian12")
    message(STATUS "Building for Debian 12/13")
else()
    message(WARNING "No build type specified. Using generic build.")
    set(BUILD_TYPE "generic")
endif()

add_definitions(-DHAVE_LIBHEIF)
add_definitions(-DLIBHEIF_HAVE_VERSION=1)  # Adjust based on your libheif version

# If building on macOS, add frameworks
if(APPLE)
    find_library(COCOA_LIB Cocoa)
    find_library(CORE_FOUNDATION_LIB CoreFoundation)
    target_link_libraries(heic_converter PRIVATE ${COCOA_LIB} ${CORE_FOUNDATION_LIB})
endif()

message(STATUS "Build type: ${BUILD_TYPE}")

# Set data directory path
set(DATA_DIR "${CMAKE_SOURCE_DIR}/data")

# First try pkg-config for libheif (often more reliable)
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(LIBHEIF_PKGCONFIG libheif QUIET)
    if(LIBHEIF_PKGCONFIG_FOUND)
        message(STATUS "libheif support: YES (via pkg-config)")
        add_definitions(-DHAVE_LIBHEIF)
        set(LIBHEIF_LIBRARIES_FOUND TRUE)
        set(USE_SYSTEM_LIBHEIF TRUE)
        set(LIBHEIF_INCLUDE_DIRS ${LIBHEIF_PKGCONFIG_INCLUDE_DIRS})
        set(LIBHEIF_LIBRARIES ${LIBHEIF_PKGCONFIG_LIBRARIES})
        set(LIBHEIF_LINK_FLAGS ${LIBHEIF_PKGCONFIG_LDFLAGS})
    endif()
endif()

# If pkg-config didn't work, try find_package
if(NOT LIBHEIF_LIBRARIES_FOUND)
    find_package(libheif QUIET)
    if(libheif_FOUND)
        message(STATUS "libheif support: YES (via find_package)")
        add_definitions(-DHAVE_LIBHEIF)
        set(LIBHEIF_LIBRARIES_FOUND TRUE)
        set(USE_SYSTEM_LIBHEIF TRUE)
        # Try to get libraries from target if available
        if(TARGET libheif::libheif)
            get_target_property(LIBHEIF_LIBRARIES libheif::libheif IMPORTED_LOCATION)
            if(NOT LIBHEIF_LIBRARIES)
                # Fallback to just linking the target
                set(LIBHEIF_TARGET libheif::libheif)
            endif()
        else()
            # Try to find the library manually
            find_library(LIBHEIF_LIB heif)
            if(LIBHEIF_LIB)
                set(LIBHEIF_LIBRARIES ${LIBHEIF_LIB})
            endif()
        endif()
    endif()
endif()

# Last resort: manual find
if(NOT LIBHEIF_LIBRARIES_FOUND)
    find_path(LIBHEIF_INCLUDE_DIR libheif/heif.h)
    find_library(LIBHEIF_LIB NAMES heif)
    if(LIBHEIF_INCLUDE_DIR AND LIBHEIF_LIB)
        message(STATUS "libheif support: YES (via manual find)")
        add_definitions(-DHAVE_LIBHEIF)
        set(LIBHEIF_LIBRARIES_FOUND TRUE)
        set(USE_SYSTEM_LIBHEIF TRUE)
        set(LIBHEIF_INCLUDE_DIRS ${LIBHEIF_INCLUDE_DIR})
        set(LIBHEIF_LIBRARIES ${LIBHEIF_LIB})
    else()
        message(WARNING "libheif not found. Using fallback decoder.")
        set(LIBHEIF_LIBRARIES_FOUND FALSE)
        set(USE_SYSTEM_LIBHEIF FALSE)
    endif()
endif()

# Find other required libraries
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)

# Add definitions for PNG and JPEG
add_definitions(-DHAVE_PNG -DHAVE_JPEG)
message(STATUS "PNG support: YES (via libpng)")
message(STATUS "JPEG support: YES (via libjpeg)")

# WebP library
find_path(WEBP_INCLUDE_DIR webp/encode.h)
find_library(WEBP_LIBRARY NAMES webp)
find_library(WEBP_DEMUX_LIBRARY NAMES webpdemux)
find_library(WEBP_MUX_LIBRARY NAMES webpmux)

if(WEBP_INCLUDE_DIR AND WEBP_LIBRARY)
    message(STATUS "WebP support: YES")
    add_definitions(-DHAVE_WEBP)
    set(WEBP_LIBRARIES_FOUND TRUE)
    set(WEBP_INCLUDE_DIRS ${WEBP_INCLUDE_DIR})
    set(WEBP_LIBRARIES ${WEBP_LIBRARY} ${WEBP_DEMUX_LIBRARY} ${WEBP_MUX_LIBRARY})
else()
    message(WARNING "WebP library not found. WebP output will not be available.")
    set(WEBP_LIBRARIES_FOUND FALSE)
endif()

# TIFF library
find_path(TIFF_INCLUDE_DIR tiffio.h)
find_library(TIFF_LIBRARY NAMES tiff)

if(TIFF_INCLUDE_DIR AND TIFF_LIBRARY)
    message(STATUS "TIFF support: YES")
    add_definitions(-DHAVE_TIFF)
    set(TIFF_LIBRARIES_FOUND TRUE)
    set(TIFF_INCLUDE_DIRS ${TIFF_INCLUDE_DIR})
    set(TIFF_LIBRARIES ${TIFF_LIBRARY})
else()
    message(WARNING "TIFF library not found. TIFF output will not be available.")
    set(TIFF_LIBRARIES_FOUND FALSE)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
if(LIBHEIF_LIBRARIES_FOUND AND LIBHEIF_INCLUDE_DIRS)
    include_directories(${LIBHEIF_INCLUDE_DIRS})
endif()
if(WEBP_LIBRARIES_FOUND AND WEBP_INCLUDE_DIRS)
    include_directories(${WEBP_INCLUDE_DIRS})
endif()
if(TIFF_LIBRARIES_FOUND AND TIFF_INCLUDE_DIRS)
    include_directories(${TIFF_INCLUDE_DIRS})
endif()

# Add source files
set(SOURCES
    src/main.cpp
    src/converter.cpp
    src/image_processor.cpp
    src/heic_decoder.cpp
    src/format_encoder.cpp
    src/batch_processor.cpp
    src/file_utils.cpp
    src/logger.cpp
    src/config.cpp
    src/metadata_handler.cpp
)

# Add executable
add_executable(heic_converter ${SOURCES})

# Test program for panorama debugging
add_executable(test_panorama
    test/test_panorama.cpp
    src/heic_decoder.cpp
    src/file_utils.cpp
    src/logger.cpp
)

# Link with the same libraries as the main executable
target_link_libraries(test_panorama PRIVATE heif PNG::PNG JPEG::JPEG m)

if(WEBP_LIBRARIES_FOUND)
    target_link_libraries(test_panorama PRIVATE ${WEBP_LIBRARIES})
endif()

if(TIFF_LIBRARIES_FOUND)
    target_link_libraries(test_panorama PRIVATE ${TIFF_LIBRARIES})
endif()

set_target_properties(test_panorama PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    OUTPUT_NAME test_panorama
)

# Link core libraries
target_link_libraries(heic_converter PRIVATE PNG::PNG JPEG::JPEG)

# LibHEIF - Use the imported target created by find_package if available
if(LIBHEIF_LIBRARIES_FOUND)
    if(DEFINED LIBHEIF_TARGET)
        target_link_libraries(heic_converter PRIVATE ${LIBHEIF_TARGET})
        message(STATUS "Linked libheif using target '${LIBHEIF_TARGET}'")
    elseif(LIBHEIF_LIBRARIES)
        target_link_libraries(heic_converter PRIVATE ${LIBHEIF_LIBRARIES})
        message(STATUS "Linked libheif via library list: ${LIBHEIF_LIBRARIES}")
    else()
        # Final fallback: just link with 'heif'
        target_link_libraries(heic_converter PRIVATE heif)
        message(STATUS "Linked libheif using default library name: heif")
    endif()
endif()

# WebP libraries
if(WEBP_LIBRARIES_FOUND)
    target_link_libraries(heic_converter PRIVATE ${WEBP_LIBRARIES})
endif()

# TIFF libraries (if found)
if(TIFF_LIBRARIES_FOUND)
    target_link_libraries(heic_converter PRIVATE ${TIFF_LIBRARIES})
endif()

# System math library
target_link_libraries(heic_converter PRIVATE m)

# Set output directory
set_target_properties(heic_converter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    OUTPUT_NAME heic_converter
)

message(STATUS "Build configuration complete.")
message(STATUS "Target: heic_converter")
message(STATUS "Build type: ${BUILD_TYPE}")
message(STATUS "System libheif: ${USE_SYSTEM_LIBHEIF}")
message(STATUS "Version: 1.1.0 - Metadata preservation enabled")  # NEW