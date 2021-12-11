cmake_minimum_required(VERSION 3.17)
project(lib_app_version)

execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE)

add_library(app_version INTERFACE)
target_include_directories(app_version
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/public)
target_compile_definitions(app_version INTERFACE
        VERSION_MAJOR=1
        VERSION_MINOR=0
        VERSION_PATCH=2
        COMMIT_HASH="${GIT_COMMIT_HASH}")