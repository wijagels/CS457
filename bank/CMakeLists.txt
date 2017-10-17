cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")

project(Bank VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-overflow=5 -Wundef)

add_library(bank src/common.cpp)
add_executable(branch src/branch.cpp src/channel.cpp src/server.cpp)
add_subdirectory(proto)
add_library(Bank::proto ALIAS proto)
add_library(Bank::bank ALIAS bank)

find_package(PkgConfig REQUIRED)
pkg_search_module(Zmq REQUIRED IMPORTED_TARGET libzmq)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

target_include_directories(bank PRIVATE include)
target_include_directories(branch PRIVATE include)

target_link_libraries(bank PUBLIC Boost::boost PkgConfig::Zmq)
target_link_libraries(branch PUBLIC Bank::bank Bank::proto)

set_target_properties(bank branch PROPERTIES CXX_CLANG_TIDY clang-tidy LINK_WHAT_YOU_USE TRUE)

set(PROJECT_TEST_NAME bank-test)
add_executable(${PROJECT_TEST_NAME} test/branch_test.cpp)

enable_testing()
find_package(GTest REQUIRED)
target_link_libraries(${PROJECT_TEST_NAME} PUBLIC GTest::GTest GTest::Main bank)
target_include_directories(${PROJECT_TEST_NAME} PUBLIC include)
add_test(test_chord ${PROJECT_TEST_NAME})
