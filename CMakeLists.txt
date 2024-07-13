cmake_minimum_required(VERSION 3.11)

project(tv_remote C)

add_executable(remote 
    main.c
    state_machine/TvRemoteSm.c 
)
set_property(TARGET remote PROPERTY C_STANDARD 11)
target_include_directories(remote PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})