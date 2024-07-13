cmake_minimum_required(VERSION 3.11)

project(tv_remote C)

include(FindCurses)
find_package(Curses REQUIRED)

add_executable(remote 
    main.c
    state_machine/ButtonSm.c
    state_machine/TvRemoteSm.h )
set_property(TARGET remote PROPERTY C_STANDARD 11)
target_link_libraries(remote -lcurses)
target_include_directories(remote PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})