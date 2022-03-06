cmake_minimum_required(VERSION 3.21)

project(ecs LANGUAGES CXX)

# Configuration for log4cplus
set(UNICODE OFF CACHE BOOL "Build with tchar = wchar_t")
set(LOG4CPLUS_ENABLE_DECORATED_LIBRARY_NAME OFF CACHE BOOL "Turns on resulting file name decoration for static and UNICODE builds.")
set(WITH_UNIT_TESTS OFF CACHE BOOL "Enable unit tests")
set(LOG4CPLUS_BUILD_LOGGINGSERVER OFF CACHE BOOL "Build the logging server.")
set(LOG4CPLUS_BUILD_TESTING OFF CACHE BOOL "Build the test suite.")
#add_subdirectory(log4cplus)
