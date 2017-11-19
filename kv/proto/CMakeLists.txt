find_package(Protobuf REQUIRED)

protobuf_generate_cpp(CLIENT_PROTO_SRCS CLIENT_PROTO_HDRS client.proto)
add_library(client-proto ${CLIENT_PROTO_SRCS} ${CLIENT_PROTO_HDRS})
target_compile_definitions(client-proto PUBLIC PROTOBUF_INLINE_NOT_IN_HEADERS=0)
target_link_libraries(client-proto INTERFACE protobuf::libprotobuf)
target_include_directories(client-proto SYSTEM INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
    )
add_library(KVStore::client-proto ALIAS client-proto)

protobuf_generate_cpp(SERVER_PROTO_SRCS SERVER_PROTO_HDRS server.proto)
add_library(server-proto ${SERVER_PROTO_SRCS} ${SERVER_PROTO_HDRS})
target_compile_definitions(server-proto PUBLIC PROTOBUF_INLINE_NOT_IN_HEADERS=0)
target_link_libraries(server-proto INTERFACE protobuf::libprotobuf)
target_include_directories(server-proto SYSTEM INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
    )
add_library(KVStore::server-proto ALIAS server-proto)

# Who the hell knows if this even works
install(TARGETS client-proto EXPORT client-proto-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
    )
INSTALL(FILES
    ${CLIENT_PROTO_HDRS}
    DESTINATION include
    )
install(EXPORT client-proto-targets
    FILE client-proto-targets.cmake
    NAMESPACE KVStore::
    DESTINATION lib/cmake/KVStore-client-proto
    )

install(TARGETS server-proto EXPORT server-proto-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
    )
INSTALL(FILES
    ${SERVER_PROTO_HDRS}
    DESTINATION include
    )
install(EXPORT server-proto-targets
    FILE server-proto-targets.cmake
    NAMESPACE KVStore::
    DESTINATION lib/cmake/KVStore-server-proto
    )
