all:P2PDownload
P2PDownload:main.cpp
	g++ -std=c++11 $^ -o $@ -lboost_filesystem -lboost_system -lpthread -lboost_thread
