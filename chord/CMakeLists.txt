cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")

set(PROJECT_NAME_STR chord)
project(${PROJECT_NAME_STR} VERSION 0.1 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(${PROJECT_NAME_STR} src/chord_constants.cpp src/chord_types.cpp src/FileStore.cpp src/sha256.cpp)
add_executable(server src/FileStore_server.cpp src/netutils.cpp)

if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(${PROJECT_NAME_STR}
        PUBLIC
        -Wall -Wextra -Werror -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wswitch-default -Wundef
        )
    target_compile_options(server
        PUBLIC
        -Wall -Wextra -Werror -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wswitch-default -Wundef
        )
endif()

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Thrift REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)

target_include_directories(${PROJECT_NAME_STR} SYSTEM PUBLIC include ${THRIFT_INCLUDE_DIR} ${BOOST_INCLUDE_DIRS})
target_include_directories(server PUBLIC include)

target_link_libraries(${PROJECT_NAME_STR} PUBLIC OpenSSL::Crypto ${THRIFT_LIBS})
target_link_libraries(server PUBLIC Threads::Threads Boost::filesystem Boost::system ${PROJECT_NAME_STR})

set(PROJECT_TEST_NAME ${PROJECT_NAME_STR}_test)
add_executable(${PROJECT_TEST_NAME} test/chord_test.cpp)

enable_testing()
find_package(GTest REQUIRED)
target_link_libraries(${PROJECT_TEST_NAME} PUBLIC GTest::GTest GTest::Main ${PROJECT_NAME_STR})
target_include_directories(${PROJECT_TEST_NAME} PUBLIC include ${GTEST_INCLUDE_DIRS})
add_test(test_chord ${PROJECT_TEST_NAME})

target_compile_options(${PROJECT_TEST_NAME}
    PUBLIC
    -Wall -Wextra -Werror -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wswitch-default -Wundef
    )
