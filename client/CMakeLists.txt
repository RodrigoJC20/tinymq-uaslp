cmake_minimum_required(VERSION 3.10)
project(tinymq_client VERSION 0.1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost (only need system and thread components for ASIO)
find_package(Boost REQUIRED COMPONENTS system thread)

# Include directories - including parent src directory to reuse packet definitions
include_directories(${Boost_INCLUDE_DIRS} ../src)

# Add all source files
file(GLOB_RECURSE CLIENT_SOURCES "src/*.cpp")
file(GLOB COMMON_SOURCES "../src/packet.cpp")

# Create executable
add_executable(tinymq_client ${CLIENT_SOURCES} ${COMMON_SOURCES})

# Link libraries
target_link_libraries(tinymq_client PRIVATE ${Boost_LIBRARIES}) 