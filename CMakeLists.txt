cmake_minimum_required(VERSION 3.20)
project(AccessibilityGamingAssistant VERSION 1.0.0 LANGUAGES CXX)

# =============================================================================
# C++ Standard
# =============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================================================================
# Build Configuration
# =============================================================================
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    message(STATUS "Setting build type to 'Release' as none was specified.")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =============================================================================
# Qt6 Configuration
# =============================================================================
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Help CMake find Qt6 - add common installation paths
list(APPEND CMAKE_PREFIX_PATH
    "C:/Qt/6.10.2/msvc2022_64"
    "C:/Qt/6.9.0/msvc2022_64"
    "C:/Qt/6.8.0/msvc2022_64"
    "C:/Qt/6.7.0/msvc2022_64"
    "C:/Qt/6.6.2/msvc2022_64"
    "C:/Qt/6.6.0/msvc2022_64"
    "C:/Qt/6.5.0/msvc2019_64"
    "D:/Qt/6.10.2/msvc2022_64"
    "D:/Qt/6.8.0/msvc2022_64"
    "D:/Qt/6.6.2/msvc2022_64"
    "$ENV{USERPROFILE}/Qt/6.10.2/msvc2022_64"
    "$ENV{USERPROFILE}/Qt/6.8.0/msvc2022_64"
    "$ENV{USERPROFILE}/Qt/6.6.2/msvc2022_64"
    "$ENV{Qt6_DIR}"
    "$ENV{QT_DIR}"
)

# Find Qt6 - REQUIRED
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
if(Qt6_FOUND)
    message(STATUS "Found Qt6 version: ${Qt6_VERSION}")
    message(STATUS "Qt6 location: ${Qt6_DIR}")
else()
    message(FATAL_ERROR "Qt6 not found! Please install Qt6 or set Qt6_DIR. Example:\n"
        "  cmake -DQt6_DIR=\"C:/Qt/6.x.x/msvc2022_64/lib/cmake/Qt6\" ..")
endif()

# =============================================================================
# OpenCV Configuration
# =============================================================================
# Help CMake find OpenCV - add vcpkg paths
list(APPEND CMAKE_PREFIX_PATH
    "C:/vcpkg/installed/x64-windows/share/opencv4"
    "D:/vcpkg/installed/x64-windows/share/opencv4"
    "$ENV{VCPKG_ROOT}/installed/x64-windows/share/opencv4"
    "$ENV{USERPROFILE}/vcpkg/installed/x64-windows/share/opencv4"
    "C:/opencv/build"
    "D:/opencv/build"
)

find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui videoio imgcodecs)
if(OpenCV_FOUND)
    message(STATUS "Found OpenCV version: ${OpenCV_VERSION}")
    message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
    message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "OpenCV not found! Please install via vcpkg:\n"
        "  vcpkg install opencv4:x64-windows\n"
        "Or set OpenCV_DIR manually.")
endif()

# =============================================================================
# Source Files
# =============================================================================
set(CORE_SOURCES
    src/core/ScreenCapture.cpp
    src/core/ColorDetection.cpp
    src/core/MouseController.cpp
    src/core/Tracker.cpp
    src/core/Overlay.cpp
)

set(UI_SOURCES
    src/ui/MainWindow.cpp
    src/ui/ColorPicker.cpp
    src/ui/AdvancedColorPicker.cpp
)

set(UTILS_SOURCES
    src/utils/ConfigManager.cpp
    src/utils/TranslationManager.cpp
    src/utils/StatsTracker.cpp
)

set(CORE_HEADERS
    include/core/ScreenCapture.h
    include/core/ColorDetection.h
    include/core/MouseController.h
    include/core/Tracker.h
    include/core/Overlay.h
)

set(UI_HEADERS
    include/ui/MainWindow.h
    include/ui/ColorPicker.h
    include/ui/AdvancedColorPicker.h
)

set(UTILS_HEADERS
    include/utils/ConfigManager.h
    include/utils/TranslationManager.h
    include/utils/StatsTracker.h
)

set(ALL_SOURCES
    src/main.cpp
    ${CORE_SOURCES}
    ${UI_SOURCES}
    ${UTILS_SOURCES}
)

set(ALL_HEADERS
    ${CORE_HEADERS}
    ${UI_HEADERS}
    ${UTILS_HEADERS}
)

# =============================================================================
# Create Executable
# =============================================================================
add_executable(${PROJECT_NAME} WIN32
    ${ALL_SOURCES}
    ${ALL_HEADERS}
    resources/resources.qrc
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

# =============================================================================
# Link Libraries
# =============================================================================
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    ${OpenCV_LIBS}
)

# =============================================================================
# Windows Specific Settings
# =============================================================================
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        user32
        gdi32
        dwmapi
    )
    
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    if(MSVC)
        set_property(TARGET ${PROJECT_NAME} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
        )
        # Suppress some common warnings
        target_compile_options(${PROJECT_NAME} PRIVATE /wd4251 /wd4275)
    endif()
endif()

# =============================================================================
# Compiler Warnings
# =============================================================================
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# =============================================================================
# Install Rules
# =============================================================================
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# =============================================================================
# Summary
# =============================================================================
message(STATUS "")
message(STATUS "=== Build Configuration Summary ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Qt6 Version: ${Qt6_VERSION}")
message(STATUS "OpenCV Version: ${OpenCV_VERSION}")
message(STATUS "===================================")
message(STATUS "")
