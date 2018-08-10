cmake_minimum_required(VERSION 3.10)
project(dodou2)

set(GCC_COVERAGE_COMPILE_FLAGS "-D_POSIX_C_SOURCE=200112L")

add_definitions(-Wall -std=c99)

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
include_directories(lib/cmocka/include)


find_library(CMOCKA_LIBRARY lib/cmocka/build/src)

add_subdirectory(lib/googletest)
link_directories({CMAKE_SOURCE_DIR}/lib)

add_executable(testPduCreator
        utils/pduCommon.c
        utils/pduCreator.c
        utils/test/test_pduCreator.cc)

add_executable(client
        client/client.c
        client/clientSession.c
        utils/dod_socket.c
        utils/dod_socket.h
        utils/pduCommon.c
        utils/pduReader.c
        utils/pduCreator.c
        utils/socketReaderAPI.c
        utils/socketReaderAPI.h)

add_executable(clientTest
        client/test/client_test.c
        client/clientSession.c
        client/client.c
        utils/pduCommon.c
        utils/pduCreator.c
        utils/mocks/dod_socketMock.c
        utils/mocks/pduReaderMock.c
        utils/socketReaderAPI.c)

add_executable(testPduReader
        utils/pduCommon.c
        utils/pduReader.c
        utils/pduCreator.c
        utils/test/test_pduReader.cc)


add_library(UTILS ${UTILS_SOURCE})

target_link_libraries(testPduCreator ${PThreadLib} ${CMAKE_SOURCE_DIR}/lib/libgtest.a)
target_link_libraries(testPduReader ${PThreadLib} ${CMAKE_SOURCE_DIR}/lib/libgtest.a)
target_link_libraries(client ${PThreadLib} UTILS)
target_link_libraries(clientTest ${CMOCKA_LIBRARY})
target_link_libraries(testPduReader ${CMOCKA_LIBRARY})

target_compile_options(clientTest PRIVATE "-D_POSIX_C_SOURCE=200112L")
target_compile_options(testPduReader PRIVATE "-D_POSIX_C_SOURCE=200112L")

add_test(clientTest ${CMAKE_CURRENT_BINARY_DIR}/clientTest)

