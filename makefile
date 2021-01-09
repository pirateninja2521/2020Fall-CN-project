all: server
	
server: new_SocketServer.c mainpage.html
	gcc new_SocketServer.c -lpthread -o server
