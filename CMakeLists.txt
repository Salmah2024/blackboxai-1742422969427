# Minimum CMake version
cmake_minimum_required(VERSION 3.1)

# Project name
project(5G_NR_V2X_Simulation)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check for required environment variables
if(NOT DEFINED ENV{OMNETPP_ROOT})
    message(FATAL_ERROR "OMNETPP_ROOT environment variable is not set")
endif()

if(NOT DEFINED ENV{INET_ROOT})
    message(FATAL_ERROR "INET_ROOT environment variable is not set")
endif()

if(NOT DEFINED ENV{VEINS_ROOT})
    message(FATAL_ERROR "VEINS_ROOT environment variable is not set")
endif()

if(NOT DEFINED ENV{SIMU5G_ROOT})
    message(FATAL_ERROR "SIMU5G_ROOT environment variable is not set")
endif()

if(NOT DEFINED ENV{SUMO_ROOT})
    message(FATAL_ERROR "SUMO_ROOT environment variable is not set")
endif()

# Set paths from environment variables
set(OMNETPP_ROOT $ENV{OMNETPP_ROOT})
set(INET_ROOT $ENV{INET_ROOT})
set(VEINS_ROOT $ENV{VEINS_ROOT})
set(SIMU5G_ROOT $ENV{SIMU5G_ROOT})
set(SUMO_ROOT $ENV{SUMO_ROOT})

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OMNETPP_ROOT}/include
    ${INET_ROOT}/src
    ${VEINS_ROOT}/src
    ${SIMU5G_ROOT}/src
    ${SUMO_ROOT}/include
)

# Find source files
file(GLOB_RECURSE SOURCES
    "src/*.cc"
    "src/*.h"
    "ned/*.ned"
)

# Create NED folders list
set(NED_FOLDERS
    ${CMAKE_CURRENT_SOURCE_DIR}/ned
    ${INET_ROOT}/src
    ${VEINS_ROOT}/src
    ${SIMU5G_ROOT}/src
)

# Convert NED folders list to string with semicolon separator
string(REPLACE ";" ";" NED_FOLDERS_STR "${NED_FOLDERS}")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${OMNETPP_ROOT}/lib/liboppmain.a
    ${OMNETPP_ROOT}/lib/liboppenvir.a
    ${OMNETPP_ROOT}/lib/libopplayout.a
    ${OMNETPP_ROOT}/lib/liboppcommon.a
    ${INET_ROOT}/src/INET
    ${VEINS_ROOT}/src/veins
    ${SIMU5G_ROOT}/src/simu5g
)

# Set compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    WITH_INET
    WITH_VEINS
    WITH_SIMU5G
)

# Set compile options
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -pedantic
    -fPIC
)

# Custom target for running simulation
add_custom_target(run
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} -u Cmdenv -f ${CMAKE_CURRENT_SOURCE_DIR}/simulations/omnetpp.ini
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Custom target for running with GUI
add_custom_target(run-gui
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} -u Qtenv -f ${CMAKE_CURRENT_SOURCE_DIR}/simulations/omnetpp.ini
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

install(DIRECTORY simulations/
    DESTINATION share/${PROJECT_NAME}/simulations
)

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration summary")
message(STATUS "=====================")
message(STATUS "Project name      : ${PROJECT_NAME}")
message(STATUS "OMNeT++ root     : ${OMNETPP_ROOT}")
message(STATUS "INET root        : ${INET_ROOT}")
message(STATUS "Veins root       : ${VEINS_ROOT}")
message(STATUS "Simu5G root      : ${SIMU5G_ROOT}")
message(STATUS "SUMO root        : ${SUMO_ROOT}")
message(STATUS "Build type       : ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler     : ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ flags        : ${CMAKE_CXX_FLAGS}")
message(STATUS "")