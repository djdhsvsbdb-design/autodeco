cmake_minimum_required(VERSION 3.15)
project(Alt3rDECO)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/ModMain.cpp
    src/ui/AIDecorateLayer.cpp
    src/engine/AIDecorationEngine.cpp
    src/engine/GeometryAnalyzer.cpp
    src/engine/PatternDetector.cpp
    src/presets/AIPresetManager.cpp
    src/presets/AIPresetData.cpp
)

# Header files
set(HEADERS
    src/ui/AIDecorateLayer.hpp
    src/engine/AIDecorationEngine.hpp
    src/engine/GeometryAnalyzer.hpp
    src/engine/PatternDetector.hpp
    src/presets/AIPresetManager.hpp
    src/presets/AIPresetData.hpp
)

# Create shared library (DLL)
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Set output name
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "Alt3rDECO"
    PREFIX ""
    SUFFIX ".dll"
)
