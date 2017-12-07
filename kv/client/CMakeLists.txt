find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

add_executable(client src/client.cpp)
target_include_directories(client PUBLIC include)
target_link_libraries(client PUBLIC Boost::boost Boost::system Threads::Threads KVStore::client-proto KVStore::Messaging KVStore::Channel)

# set_target_properties(client PROPERTIES CXX_CLANG_TIDY clang-tidy)
