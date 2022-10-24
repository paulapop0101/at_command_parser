cmake_minimum_required(VERSION 3.21)
project(AT_parser C)

set(CMAKE_C_STANDARD 99)

add_executable(AT_parser main.c at_command_parser.c at_command_parser.h)
