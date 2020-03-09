all:
	g++ -g -o tpoll src/ThreadPool.cc  src/main.cc -lpthread -std=c++11 

clean:
	rm tpoll
