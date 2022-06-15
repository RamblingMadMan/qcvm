add_executable(qcvm-test main.cpp)

target_link_libraries(qcvm-test PRIVATE qcvm Catch2)
