cmake_minimum_required(VERSION 3.20.1)

project(esp32-lora-performance-evaluation)

set(CMAKE_CXX_STANDARD 17)

# Define the target executable
add_executable(${PROJECT_NAME} main.cpp)