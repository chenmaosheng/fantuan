main: main.cpp
	gcc -o main network.cpp context.cpp connection.cpp acceptor.cpp worker.cpp main.cpp -lstdc++ -O0 -g3
