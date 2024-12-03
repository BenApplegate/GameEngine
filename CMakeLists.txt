cmake_minimum_required(VERSION 3.29)
project(GameEngine)

set(CMAKE_CXX_STANDARD 20)

add_executable(GameEngine main.cpp)

find_package(Vulkan REQUIRED)

include_directories(${PROJECT_NAME} "include")
target_link_directories(${PROJECT_NAME} PUBLIC "lib")

target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan glfw3)