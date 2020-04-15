main: main.cpp
	gcc -o main network.cpp socket.cpp acceptor.cpp main.cpp -lstdc++
