file(GLOB SRC_LIST "*.cpp")

add_executable(ThreadPool ${SRC_LIST})

target_link_libraries(ThreadPool pthread)