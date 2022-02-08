cmake_minimum_required(VERSION 3.21)

project(ecs LANGUAGES CXX)

# Configuration for log4cplus
set(UNICODE OFF)
set(LOG4CPLUS_ENABLE_DECORATED_LIBRARY_NAME OFF)
set(WITH_UNIT_TESTS OFF)
set(LOG4CPLUS_BUILD_LOGGINGSERVER OFF)
set(LOG4CPLUS_BUILD_TESTING OFF)
add_subdirectory(log4cplus)