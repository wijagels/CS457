find_package(Protobuf REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

add_library(Channel INTERFACE)
add_library(KVStore::Channel ALIAS Channel)

target_include_directories(Channel INTERFACE include)

target_link_libraries(Channel INTERFACE Boost::boost Boost::system KVStore::Messaging protobuf::libprotobuf)
