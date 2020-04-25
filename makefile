main: main.cpp
	gcc -o main network.cpp context.cpp connection.cpp acceptor.cpp worker.cpp workerpool.cpp poller.cpp main.cpp -lstdc++ -lpthread -O0 -g3
