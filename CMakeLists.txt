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

add_subdirectory(lib/googletest)
link_directories({CMAKE_SOURCE_DIR}/lib)

add_executable(testPduCreator
        utils/pduCommon.c
        utils/pduCreator.c
        utils/test/test_pduCreator.cc)

add_executable(client
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