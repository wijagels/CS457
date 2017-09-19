cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

set(PROJECT_NAME_STR http)
set(PROJECT_TEST_NAME ${PROJECT_NAME_STR}_test)
project(${PROJECT_NAME_STR} VERSION 0.1 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(${PROJECT_NAME_STR} src/socket.cpp src/http.cpp src/mimedb.cpp src/logger.cpp)
add_executable(${PROJECT_TEST_NAME} test/test.cpp)
add_executable(server src/main.cpp src/server.cpp)

target_compile_features(${PROJECT_NAME_STR} PRIVATE cxx_constexpr cxx_variadic_templates)

if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(${PROJECT_NAME_STR}
        PUBLIC
        -g -Wall -Wextra -Werror -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror
        )
    target_compile_options(${PROJECT_TEST_NAME}
        PUBLIC
        -Wall -Wextra -Werror -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror
        )
    target_compile_options(server
        PUBLIC
        -g -Wall -Wextra -Werror -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror
        )
endif()

find_package(Threads REQUIRED)


target_include_directories(${PROJECT_NAME_STR} PUBLIC include)
target_include_directories(server PUBLIC include)

target_link_libraries(${PROJECT_NAME_STR} PRIVATE libstdc++fs.a)
target_link_libraries(server PRIVATE ${PROJECT_NAME_STR} libstdc++fs.a Threads::Threads)

enable_testing()
find_package(GTest REQUIRED)
target_link_libraries(${PROJECT_TEST_NAME} PRIVATE GTest::GTest GTest::Main ${PROJECT_NAME_STR})
target_include_directories(${PROJECT_TEST_NAME} PUBLIC include ${GTEST_INCLUDE_DIRS})
add_test(TestServer ${PROJECT_TEST_NAME})
