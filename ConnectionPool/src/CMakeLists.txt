# aux_source_directory(. SRC_LIST)
file(GLOB SRC_LIST "*.cpp")

add_executable(test ${SRC_LIST})

# find_package(Protobuf REQUIRED)

# target_link_libraries(test ${PROTOBUF_LIBRARIES})


target_link_libraries(test mysqlclient pthread)