cmake_minimum_required(VERSION 3.11)

project(tv_remote C)

include(FindCurses)
find_package(Curses REQUIRED)

add_executable(remote main.c)
set_property(TARGET remote PROPERTY C_STANDARD 11)
target_link_libraries(remote -lcurses)