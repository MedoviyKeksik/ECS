cmake_minimum_required(VERSION 3.21)

project(ecs LANGUAGES CXX)

file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h
                               ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

set(SOURCES ${HEADER_FILES} ${SOURCE_FILES})

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC log4cplus)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/memory/allocators
                                                   ${CMAKE_CURRENT_SOURCE_DIR}/memory
                                                   ${CMAKE_CURRENT_SOURCE_DIR}/event
                                                   ${CMAKE_CURRENT_SOURCE_DIR}/util)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
