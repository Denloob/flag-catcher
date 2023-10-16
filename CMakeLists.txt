# Minimum CMake version required, we'll just use the latest version.
cmake_minimum_required(VERSION 3.22)
# Project name, version and description
project(discord-bot VERSION 1.0 DESCRIPTION "A discord bot")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake /usr/lib/cmake/vtk)

file(GLOB SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Find our pre-installed DPP package (using FindDPP.cmake).
find_package(DPP REQUIRED)

# Find libCURL package
find_package(CURL REQUIRED)

# Find JsonCpp package
find_package(JsonCpp REQUIRED)

# Add SQLiteCpp
set(SQLITECPP_RUN_CPPCHECK OFF CACHE BOOL "" FORCE)
set(SQLITECPP_RUN_CPPLINT OFF CACHE BOOL "" FORCE)
add_subdirectory(SQLiteCpp)

# Link the libraries
target_link_libraries(${PROJECT_NAME}
    ${DPP_LIBRARIES}
    ${JsonCpp_LIBRARIES}
    ${CURL_LIBRARIES}
    SQLiteCpp
)

# Include the DPP directories.
target_include_directories(${PROJECT_NAME} PRIVATE
    ${DPP_INCLUDE_DIR}
)

# Include libCURL headers
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CURL_INCLUDE_DIR}
)

# Include JsonCpp headers
target_include_directories(${PROJECT_NAME} PRIVATE
    ${JsonCpp_INCLUDE_DIRS}
)

# compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# Set C++ version
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Set the output directory to bin/PROJECT_NAME
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/"
)
