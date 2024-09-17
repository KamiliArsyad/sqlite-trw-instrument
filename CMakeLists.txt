cmake_minimum_required(VERSION 3.29)
project(sqlite_rw_instrument C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(sqlite_rw_instrument
    shell.c
    sqlite3.c)

add_definitions(-DSQLITE_DEBUG)

target_link_libraries(sqlite_rw_instrument pthread dl)
