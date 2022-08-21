cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 20)

project(amonRPC CXX C ASM)

# set CXXFALGS
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CXX_FLAGS
        /std:c++20
        /await:strict
        /EHa
        )
else()
    set(CXX_FLAGS
        -std=c++20
        -D_GLIBCXX_USE_CXX11_ABI=1
        -Wno-deprecated-register
        -D_FILE_OFFSET_BITS=64
        -fPIC
        -Wall
        -Werror
        -D__STDC_LIMIT_MACROS
        -lpthread
        --copy-dt-needed-entries -lA
        -g
        )
endif()



include_directories(include/asio)

add_executable(RPC_demo_server test/rpc_server_test.cpp)
target_link_libraries(RPC_demo_server async_simple libpthread.so.0)

add_executable(RPC_demo_client test/rpc_client/client.cpp)
target_link_libraries(RPC_demo_client libpthread.so.0)


# add_custom_command(
#         TARGET ReadFiles POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_directory
#                 ${CMAKE_SOURCE_DIR}/demo_example/Input
#                 ${CMAKE_CURRENT_BINARY_DIR}/Input)

