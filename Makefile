CC=gcc
C++ = g++

SRCSSERVER = $(wildcard server/*)
SRCSCLIENT = $(wildcard client/*)
SRCSUTILS  = $(wildcard utils/*.c)
TESTUTILS = $(wildcard utils/test/*.cc)
GTEST_DIR = $(wildcard lib/googletest)

GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)


OUT_MAINSERVER	=	out/chatServer
OUT_MAINCLIENT	=	out/chatClient
OUT_UTILSTEST 	= out/utilstest


CFLAGS = -std=c99 -Wall -D_POSIX_C_SOURCE=200112L \
				 -lpthread -pthread #-Werror  

CPPFLAGS += -isystem $(GTEST_DIR)/include
CXXFLAGS += -g -Wall -Wextra -pthread
ARFLAGS = -rv

#	-Wextra -Wmissing-declarations\
# -Werror-implicit-function-declaration\
#	-Wreturn-type -Wparentheses -Wunused -Wold-style-definition\
#	-Wundef -Wshadow -Wstrict-prototypes -Wswitch-default\
#	-Wstrict-prototypes -Wunreachable-code 



client: $(SRCSCLIENT) utils.o  
	$(CC) $(CFLAGS) $(SRCSCLIENT) $(SRCSUTILS) -o $(OUT_MAINCLIENT) -g

server:	$(SRCSSERVER) utils.o  
	$(CC) $(CFLAGS) $(SRCSSERVER) $(SRCSUTILS) -o $(OUT_MAINSERVER) -g

utils.o : $(SRCSUTILS)  
	$(CC) $(CFLAGS) $(SRCSUTILS)  -g

testUtils.o: $(TESTUTILS) $(GTEST_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(TESTUTILS) -o out/$@

testUtils: $(TESTUTILS) gtest_main.a utils.o testUtils.o
	$(C++) $(CPPFLAGS) $(CXXFLAGS) out/* -o $(OUT_UTILSTEST)  -g

gtest-all.o : $(GTEST_SRCS_)
	$(C++) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc -o out/$@

gtest_main.o : $(GTEST_SRCS_)
	$(C++) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc -o out/$@

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) out/$@ 


#gtest.o: 
#	$(C++) $(C++FLAGS) -isystem ${GTEST_DIR}/include	$(LIBGTEST)

#gtest.o: 
#	$(C++) -isystem ${GTEST_DIR}/include -I${GTEST_DIR} \
#				-pthread -c ${GTEST_DIR}/src/gtest-all.cc 


clean:
	rm -f *.o *.o *.a out/* core

