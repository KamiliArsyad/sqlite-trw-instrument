cmake_minimum_required(VERSION 3.10)
project(sqlite_rw_instrument C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(sqlite_rw_instrument
    sqlite3.c
    shell.c
    )

add_definitions(-DSQLITE_DEBUG -DSQLITE_TRW_INSTRUMENT)

target_link_libraries(sqlite_rw_instrument pthread dl)

add_subdirectory(multithread_runner)