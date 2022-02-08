cmake_minimum_required(VERSION 3.21)

project(ecs LANGUAGES CXX)

add_subdirectory(log4cplus)

target_include_directories(${PROJECT_NAME} PUBLIC log4cplus/include)
