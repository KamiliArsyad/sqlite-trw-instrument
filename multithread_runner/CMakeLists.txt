cmake_minimum_required(VERSION 3.10)
project(MultithreadRunner CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR})

find_package(Threads REQUIRED)

add_definitions(-DSQLITE_DEBUG -DSQLITE_TRW_INSTRUMENT -DHAVE_USLEEP=1)

add_executable(full_runner full_runner.cpp ${CMAKE_SOURCE_DIR}/sqlite3.c
        ${CMAKE_SOURCE_DIR}/sqlite3_ext.h
        ${CMAKE_SOURCE_DIR}/mvtracer.c ${CMAKE_SOURCE_DIR}/sqlite3TraceAdapter.c)

add_executable(threadtest threadtest.c ${CMAKE_SOURCE_DIR}/sqlite3.c
        ${CMAKE_SOURCE_DIR}/sqlite3_ext.h
        ${CMAKE_SOURCE_DIR}/mvtracer.c ${CMAKE_SOURCE_DIR}/sqlite3TraceAdapter.c)


target_link_libraries(full_runner Threads::Threads pthread dl)
target_link_libraries(threadtest Threads::Threads pthread dl)
