WARNING=-Wall
OPTIMIZE=-O0
DEBUG=-g3
COMPILE=-c
INCLUDE=-Icommon
INLINE=-finline-functions
CFLAGS=-fno-rtti -Wreorder -Woverloaded-virtual -ansi
CC=gcc
CFLAGS_END=-lstdc++ -std=c++11 -lpthread

debug:
	$(CC) $(WARNING) $(CFLAGS) $(OPTIMIZE) $(DEBUG) $(INCLUDE) $(INLINE) -o main \
		common/utils.cpp acceptor.cpp connection.cpp context.cpp network.cpp poller.cpp \
		worker.cpp workerpool.cpp main.cpp $(CFLAGS_END)

clean:
	rm -f main

release:
	$(CC) $(WARNING) $(CFLAGS) -O3 $(INCLUDE) $(INLINE) -o main \
		common/utils.cpp acceptor.cpp connection.cpp context.cpp network.cpp poller.cpp \
		worker.cpp workerpool.cpp main.cpp $(CFLAGS_END)