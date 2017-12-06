find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(GTest REQUIRED)
include(GoogleTest)
pkg_search_module(Jsoncpp REQUIRED IMPORTED_TARGET jsoncpp)

add_library(server src/coordinator.cpp src/log.cpp src/replica.cpp src/config.cpp)
add_library(KVStore::Server ALIAS server)

target_include_directories(server PUBLIC include)

target_link_libraries(server PUBLIC Boost::boost Boost::system KVStore::server-proto KVStore::client-proto KVStore::hint-proto KVStore::Messaging KVStore::Channel stdc++fs.a PkgConfig::Jsoncpp)

add_executable(replica src/server.cpp)
target_link_libraries(replica PUBLIC server Threads::Threads)

# set_target_properties(server PROPERTIES CXX_CLANG_TIDY clang-tidy)

add_executable(log-test test/log_test.cpp)
target_link_libraries(log-test GTest::GTest GTest::Main KVStore::Server)
gtest_add_tests(
    TARGET log-test
    TEST_LIST log-tests
    )

add_executable(config-test test/config_test.cpp)
target_link_libraries(config-test GTest::GTest GTest::Main KVStore::Server)
gtest_add_tests(
    TARGET config-test
    TEST_LIST config-tests
    )
