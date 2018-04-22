CC=gcc

SRCSSERVER = $(wildcard server/*)
SRCSCLIENT = $(wildcard client/*)
SRCSUTILS  = $(wildcard utils/*.c)
TESTSUTILS = $(wildcard utils/test/*.c)

LIBUNISTR = $(/urs/lib/libunistring.so)

OUT_MAINSERVER	=	chatServer
OUT_MAINCLIENT	=	chatClient
OUT_MAINUTILS		= utilsc
OUT_UTILSTEST 	= utilstest


CFLAGS = -std=c99 -Wall -D_POSIX_C_SOURCE=200112L \
				 -lpthread -pthread -LLIBUNISTR #-Werror  

#	-Wextra -Wmissing-declarations\
# -Werror-implicit-function-declaration\
#	-Wreturn-type -Wparentheses -Wunused -Wold-style-definition\
#	-Wundef -Wshadow -Wstrict-prototypes -Wswitch-default\
#	-Wstrict-prototypes -Wunreachable-code 

server: server.o

client: client.o

utils: utils.o

testUtils: testUtils.o

client.o: $(SRCSCLIENT) utils.o  
	$(CC) $(CFLAGS) $(SRCSCLIENT) $(SRCSUTILS) -o $(OUT_MAINCLIENT) -g

server.o:	$(SRCSSERVER) utils.o  
	$(CC) $(CFLAGS) $(SRCSSERVER) $(SRCSUTILS) -o $(OUT_MAINSERVER) -g

utils.o : $(SRCSUTILS)  
	$(CC) $(CFLAGS) $(SRCSUTILS) -o $(OUT_MAINUTILS) -g

testUtils.o: $(TESTUTILS) utils.o  
	$(CC) $(CFLAGS) $(SRCSUTILS) -o $(OUT_UTILSTEST) -g


clean:
	rm -f *.o *.o core
