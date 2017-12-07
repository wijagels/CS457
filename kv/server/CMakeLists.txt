hunter_add_package(Boost COMPONENTS system)
# hunter_add_package(GTest)
hunter_add_package(jsoncpp)
find_package(Boost CONFIG REQUIRED system)
# find_package(GTest CONFIG REQUIRED)
# include(GoogleTest)
find_package(jsoncpp CONFIG REQUIRED)
find_package(Threads REQUIRED)

add_library(server src/coordinator.cpp src/log.cpp src/replica.cpp src/config.cpp)
add_library(KVStore::Server ALIAS server)

target_include_directories(server PUBLIC include)

target_link_libraries(server PUBLIC Boost::boost Boost::system KVStore::server-proto KVStore::client-proto KVStore::hint-proto KVStore::Messaging KVStore::Channel stdc++fs.a jsoncpp_lib_static)

add_executable(replica src/server.cpp)
target_link_libraries(replica PUBLIC server Threads::Threads)

# set_target_properties(server PROPERTIES CXX_CLANG_TIDY clang-tidy)

# add_executable(log-test test/log_test.cpp)
# target_link_libraries(log-test GTest::gtest GTest::main KVStore::Server)
# gtest_add_tests(
#     TARGET log-test
#     TEST_LIST log-tests
#     )

# add_executable(config-test test/config_test.cpp)
# target_link_libraries(config-test GTest::gtest GTest::main KVStore::Server)
# gtest_add_tests(
#     TARGET config-test
#     TEST_LIST config-tests
#     )
