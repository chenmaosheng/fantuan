main: main.cpp
	gcc -o main network.cpp context.cpp connection.cpp acceptor.cpp log.cpp main.cpp -lstdc++ -pthread -O0 -g3
