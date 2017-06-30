server: myserver.o main.o
	g++ -o server ./main.o ./myserver.o
myserver.o: myserver.cpp myserver.h
	g++ -g -o myserver.o -c ./myserver.cpp
main.o: main.cpp
	g++ -g -o main.o -c ./main.cpp

clean: 
	rm *.o server
