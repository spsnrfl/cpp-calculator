cmake_minimum_required(VERSION 3.20)

project(CalculatorHomework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT WIN32)
    message(FATAL_ERROR "This project currently supports Windows only")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

foreach(CONFIG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG} "${CMAKE_BINARY_DIR}")
endforeach()

add_executable(Calculator
    Calculator/Calculator.cpp
)

set(PLUGIN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/plugins")

function(configure_plugin target)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    )

    foreach(CONFIG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
        set_target_properties(${target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${CONFIG} "${PLUGIN_OUTPUT_DIR}"
        )
    endforeach()
endfunction()

add_library(PowerPlugin SHARED
    PowerPlugin/PowerPlugin.cpp
)
configure_plugin(PowerPlugin)

add_library(SinPlugin SHARED
    SinPlugin/SinPlugin.cpp
)
configure_plugin(SinPlugin)

add_library(LnPlugin SHARED
    LnPlugin/LnPlugin.cpp
)
configure_plugin(LnPlugin)