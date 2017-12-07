hunter_add_package(Protobuf)
hunter_add_package(Boost COMPONENTS system)
find_package(Protobuf CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED system)

add_library(Channel INTERFACE)
add_library(KVStore::Channel ALIAS Channel)

target_include_directories(Channel INTERFACE include)

target_link_libraries(Channel INTERFACE Boost::boost Boost::system KVStore::Messaging protobuf::libprotobuf)
