cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

set(PROJECT_NAME_STR chord)
project(${PROJECT_NAME_STR} VERSION 0.1 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(server src/chord_constants.cpp src/chord_types.cpp src/FileStore.cpp src/FileStore_server.cpp)

if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(server
        PUBLIC
        -g -Wall -Wextra -Werror -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror
        )
endif()

find_package(Threads REQUIRED)

target_include_directories(server PUBLIC include)

target_link_libraries(server PUBLIC ${PROJECT_NAME_STR} libstdc++fs.a Threads::Threads)
