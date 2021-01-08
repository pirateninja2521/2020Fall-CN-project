all: server
	
server: SocketServer.cpp mainpage.html
	g++ SocketServer.cpp -o server
