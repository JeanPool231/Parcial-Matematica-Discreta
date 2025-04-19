cmake_minimum_required(VERSION 3.10)
project(MiProyectoTGUI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)
find_package(TGUI 0.9 REQUIRED)

add_executable(mi_app main.cpp)

target_link_libraries(mi_app PRIVATE tgui sfml-graphics sfml-window sfml-system)
