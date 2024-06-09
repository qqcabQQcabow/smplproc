cmake_minimum_required(VERSION 3.12)
project(firsttry)

set(CMAKE_CXX_STANDARD 11)

find_package(TGUI REQUIRED)

add_executable(smplproc main.cpp process.cpp setting.cpp)
target_link_libraries(smplproc PRIVATE TGUI::TGUI)
