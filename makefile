all: server
	
server: SocketServer.c mainpage.html
	gcc SocketServer.c -o server
