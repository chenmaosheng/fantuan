main: main.cpp
	gcc -o main network.cpp socket.cpp context.cpp connection.cpp acceptor.cpp main.cpp -lstdc++
