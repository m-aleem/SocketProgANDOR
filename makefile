all: server_and.cpp server_or.cpp edge.cpp client.cpp server_and.h server_or.h edge.h client.h
	g++ -o server_and_exec server_and.cpp
	g++ -o server_or_exec server_or.cpp
	g++ -o edge_exec edge.cpp
	g++ -o client client.cpp

server_and: server_and_exec
	./server_and_exec

server_or: server_or_exec
	./server_or_exec

edge: edge_exec
	./edge_exec