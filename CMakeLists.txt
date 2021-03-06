cmake_minimum_required(VERSION 3.5)
project(dodou2)

set(GCC_COVERAGE_COMPILE_FLAGS "-D_POSIX_C_SOURCE=200112L")

set(CCOMPILE_OPTIONS
        -Wall -Werror -std=c99 -g)
set(CPPCOMPILE_OPTIONS)

#set(CMAKE_CXX_STANDARD 11)

set(PThreadLib -pthread)
set(UTILS_SOURCE
        utils/pduCommon.c
        utils/pduReader.c
        utils/pduCreator.c
        utils/sysCall_facade.c
        utils/socketReaderAPI.c
        utils/list.c)

set(CMOCKA_SRC lib/cmocka)


include_directories(lib/googletest/include lib/googletest/src)
include_directories(lib)
include_directories(utils)
include_directories(client)
include_directories(server)
include_directories(client/test)
include_directories(lib/cmocka/include)
include_directories(lib/cmocka/build/src)


add_subdirectory(lib/googletest)
add_subdirectory(${CMOCKA_SRC})

link_directories({CMAKE_SOURCE_DIR}/lib)
link_directories(utils)



add_executable(testPduCreator
        utils/pduCommon.c
        utils/pduCreator.c
        utils/test/test_pduCreator.cc)

add_executable(client
        client/clientMain.c
        client/client.c
        client/client.h
        client/clientSession.c
        utils/helpers.c
        utils/helpers.h
        utils/sysCall_facade.c
        utils/sysCall_facade.h
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
        utils/mocks/sysCall_facadeMock.c
        utils/mocks/pduReaderMock.c
        utils/socketReaderAPI.c
        client/test/ThreadingSolution.h utils/helpers.c utils/helpers.h)

add_executable(testPduReader
        utils/pduCommon.c
        utils/pduReader.c
        utils/pduCreator.c
        utils/test/test_pduReader.cc)

add_executable(server
        utils/pduCommon.c
        utils/pduCreator.c
        utils/pduReader.c
        utils/sysCall_facade.c
        utils/socketReaderAPI.c
        utils/socketReaderAPI.h
        utils/list.c
        utils/list.h
        utils/helpers.c
        utils/helpers.h
        server/nameServerConnection.c
        server/nameServerConnection.h
        server/server.c
        server/server.h
        server/serverMain.c
        server/clientConnection.c
        server/clientConnection.h server/inputHandler.c server/inputHandler.h)


add_library(UTILS ${UTILS_SOURCE})
#add_library(CMOCKA SHARED ${CMOCKA_SRC}/src/cmocka.c)



target_link_libraries(testPduCreator ${PThreadLib} ${CMAKE_SOURCE_DIR}/lib/libgtest.a)
target_link_libraries(testPduReader ${PThreadLib} ${CMAKE_SOURCE_DIR}/lib/libgtest.a)
target_link_libraries(client ${PThreadLib} ${UTILS})
target_link_libraries(server ${PThreadLib} ${UTILS})
target_link_libraries(clientTest ${PThreadLib} ${CMOCKA_SHARED_LIBRARY})
target_link_libraries(testPduReader ${CMOCKA_SHARED_LIBRARY})

target_compile_options(clientTest PRIVATE "-D_POSIX_C_SOURCE=200112L" ${CCOMPILE_OPTIONS})
target_compile_options(testPduReader PRIVATE ${CPPCOMPILE_OPTIONS})

add_test(clientTest ${CMAKE_CURRENT_BINARY_DIR}/clientTest)

