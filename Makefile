CC=gcc
CPP=g++

SRCSSERVER = $(wildcard server/*)
SRCSCLIENT = $(wildcard client/*)
TESTPDUCREATOR = $(wildcard utils/test/*.cc)
UTIL_DIR = utils
GTEST_DIR = $(wildcard lib/googletest)

GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

OUT_MAINSERVER	=	out/chatServer
OUT_MAINCLIENT	=	out/chatClient
OUT_UTILSTEST 	= out/testPduCreator

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

list.o: $(UTIL_DIR)/list.c $(UTIL_DIR)/list.c 
	$(CC) $(CFLAGS) -c  -o out/$@ $(SRCSLIST)   -g

pduCreator.o : $(UTIL_DIR)/pduCreator.c  $(UTIL_DIR)/pduCreator.h 
	$(CC) $(CFLAGS) -c $(UTIL_DIR)/pduCreator.c -o out/$@ -I$(UTIL_DIR)  -g

#Unit testing
testPduCreator.o: $(TESTPDUCREATOR) $(GTEST_HEADERS) pduCreator.o
	$(CXX) $(CPPFLAGS) -I$(UTIL_DIR) $(CXXFLAGS) -c $(TESTPDUCREATOR) -o out/$@

testPduCreator: testPduCreator.o pduCreator.o 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) out/testPduCreator.o out/pduCreator.o lib/libgtest.a -o $(OUT_UTILSTEST)  -g

#Google test
gtest-all.o : $(GTEST_SRCS_)
	$(CPP) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
         $(GTEST_DIR)/src/gtest-all.cc -o out/$@

gtest_main.o : $(GTEST_SRCS_)
	$(CPP) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
         $(GTEST_DIR)/src/gtest_main.cc -o out/$@


libgtest.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) lib/$@ out/gtest*

clean:
	rm -f *.o *.o *.a out/* core

