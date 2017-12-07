hunter_add_package(Protobuf)
find_package(Protobuf CONFIG REQUIRED)

add_library(Messaging INTERFACE)
add_library(KVStore::Messaging ALIAS Messaging)

target_include_directories(Messaging INTERFACE include)

target_link_libraries(Messaging INTERFACE protobuf::libprotobuf)
