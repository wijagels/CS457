cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.181.tar.gz"
    SHA1 "d1746c22942f7310fc6a45fb6cdd5b8afcd6742c"
)

project(KVStore VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-std=c++17 -Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-overflow=5 -Wundef)

enable_testing()
add_subdirectory(proto)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(net)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
