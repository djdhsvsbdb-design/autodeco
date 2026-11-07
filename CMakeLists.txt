cmake_minimum_required(VERSION 3.15)
project(AutoDecoratorAI)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/ModMain.cpp
    src/ui/AIDecorateLayer.cpp
    src/ai/ProviderRouter.cpp
    src/ai/AIService.cpp
    src/ai/DecorationPlan.cpp
)

set(HEADERS
    src/ui/AIDecorateLayer.hpp
    src/ai/ProviderRouter.hpp
    src/ai/AIService.hpp
    src/ai/DecorationPlan.hpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "AutoDecoratorAI"
    PREFIX ""
    SUFFIX ".dll"
)
