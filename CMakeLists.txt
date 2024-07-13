cmake_minimum_required(VERSION 3.23)

project(tv_remote C)

add_executable(remote main.c)
set_property(TARGET remote PROPERTY C_STANDARD 11)