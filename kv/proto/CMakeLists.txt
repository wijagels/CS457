hunter_add_package(Protobuf)
find_package(Protobuf CONFIG REQUIRED)

set(MESSAGES_PROTOS server.proto client.proto hint.proto)
set(MESSAGES_SRCS)
set(MESSAGES_HDRS)

foreach(proto_file ${MESSAGES_PROTOS})
  get_filename_component(proto_file_abs ${proto_file} ABSOLUTE)
  get_filename_component(basename ${proto_file} NAME_WE)
  set(generated_files ${basename}.pb.cc ${basename}.pb.h)

  list(APPEND MESSAGES_SRCS ${basename}.pb.cc)
  list(APPEND MESSAGES_HDRS ${CMAKE_CURRENT_BINARY_DIR}/${basename}.pb.h)

  add_custom_command(
      OUTPUT ${generated_files}
      COMMAND protobuf::protoc --cpp_out ${CMAKE_CURRENT_BINARY_DIR} -I ${CMAKE_CURRENT_SOURCE_DIR} ${proto_file_abs}
      COMMENT "Generating ${generated_files} from ${proto_file}"
      VERBATIM
  )
endforeach()

add_library(client-proto client.pb.cc client.pb.h)
target_compile_definitions(client-proto PUBLIC PROTOBUF_INLINE_NOT_IN_HEADERS=0)
target_link_libraries(client-proto PUBLIC protobuf::libprotobuf)
target_include_directories(client-proto SYSTEM INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
add_library(KVStore::client-proto ALIAS client-proto)

add_library(server-proto server.pb.cc server.pb.h)
target_compile_definitions(server-proto PUBLIC PROTOBUF_INLINE_NOT_IN_HEADERS=0)
target_link_libraries(server-proto PUBLIC protobuf::libprotobuf)
target_include_directories(server-proto SYSTEM INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
add_library(KVStore::server-proto ALIAS server-proto)

add_library(hint-proto hint.pb.cc hint.pb.h)
target_compile_definitions(hint-proto PUBLIC PROTOBUF_INLINE_NOT_IN_HEADERS=0)
target_link_libraries(hint-proto PUBLIC protobuf::libprotobuf)
target_include_directories(hint-proto SYSTEM INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
add_library(KVStore::hint-proto ALIAS hint-proto)
