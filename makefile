all: server
	
server: SocketServer.c
	gcc SocketServer.c -o server
