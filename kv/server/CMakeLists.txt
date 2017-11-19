find_package(Boost REQUIRED COMPONENTS system)
find_package(GTest REQUIRED)
include(GoogleTest)

add_library(server src/coordinator.cpp src/log.cpp src/replica.cpp)
add_library(KVStore::Server ALIAS server)

target_include_directories(server PUBLIC include)

target_link_libraries(server PUBLIC Boost::boost Boost::system KVStore::server-proto KVStore::Messaging stdc++fs.a)

set_target_properties(server PROPERTIES CXX_CLANG_TIDY clang-tidy)

add_executable(log-test test/log_test.cpp)
target_link_libraries(log-test GTest::GTest GTest::Main KVStore::Server)
gtest_add_tests(
    TARGET log-test
    TEST_LIST log-tests
    )
