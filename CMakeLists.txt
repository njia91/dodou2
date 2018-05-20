cmake_minimum_required(VERSION 3.10)
project(dodou2)


#set(CMAKE_CXX_STANDARD 11)
set(PThreadLib -pthread)

include_directories(lib/googletest/include lib/googletest/src)
include_directories(lib)
include_directories(utils)

add_subdirectory(lib/googletest)
link_directories({CMAKE_SOURCE_DIR}/lib)

add_executable(testPduCreator
        utils/pduCommon.c
        utils/pduCreator.c
        utils/test/test_pduCreator.cc)

target_link_libraries(testPduCreator ${PThreadLib} ${CMAKE_SOURCE_DIR}/lib/libgtest.a)
#target_link_libraries(testPduCreator gtest gtest_main)