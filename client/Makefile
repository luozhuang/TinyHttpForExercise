client: myclient.o main.o
	g++ myclient.o main.o -o client
myclient.o: myclient.cpp myclient.h
	g++ -g -o myclient.o -c ./myclient.cpp
main.o: main.cpp 
	g++ -g -c ./main.cpp
clean: 
	rm *.o ./client
