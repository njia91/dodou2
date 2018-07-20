cmake_minimum_required(VERSION 3.10)
project(dodou2)


#set(CMAKE_CXX_STANDARD 11)
set(PThreadLib -pthread)
set(UTILS_SOURCE
        utils/pduCommon.c
        utils/pduReader.c
        utils/pduCreator.c
        utils/list.c)


include_directories(lib/googletest/include lib/googletest/src)
include_directories(lib)
include_directories(utils)
include_directories(client)
include_directories(${CMOCKA_PUBLIC_INCLUDE_DIRS})

add_subdirectory(lib/googletest)
link_directories({CMAKE_SOURCE_DIR}/lib)

add_executable(testPduCreator
        utils/pduCommon.c
        utils/pduCreator.c
        utils/test/test_pduCreator.cc)

add_executable(client
        client/client.c
        utils/dod_socket.c
        utils/dod_socket.h)

add_executable(clientTest
        client/test/client_test.c
        client/client.c)

add_executable(testPduReader
        utils/pduCommon.c
        utils/pduReader.c
        utils/pduCreator.c
        utils/test/test_pduReader.cc)


add_library(UTILS ${UTILS_SOURCE})

target_link_libraries(testPduCreator ${PThreadLib} ${CMAKE_SOURCE_DIR}/lib/libgtest.a)
target_link_libraries(testPduReader ${PThreadLib} ${CMAKE_SOURCE_DIR}/lib/libgtest.a)
target_link_libraries(client ${PThreadLib} UTILS)
target_link_libraries(clientTest ${CMOCKA_SHARED_LIBRARY})

add_test(clientTest ${CMAKE_CURRENT_BINARY_DIR}/clientTest)