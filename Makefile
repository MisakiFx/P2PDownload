all:client server
client:client.cpp
	g++ -std=c++11 $^ -o $@ -lboost_filesystem -lboost_system -lpthread -lboost_thread
server:server.cpp
	g++ -std=c++11 $^ -o $@ -lboost_filesystem -lboost_system -lpthread
