cmake_minimum_required(VERSION 2.6)

project(rsi)
include_directories(include)
link_directories(/root/rsi/lib)
add_subdirectory(src bin)
