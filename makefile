main: main.cpp
	gcc -o main network.cpp socket.cpp acceptor.cpp connection.cpp main.cpp -lstdc++
