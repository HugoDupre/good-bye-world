all: client server

functions.o: functions.c functions.h
	colorgcc -c functions.c -o functions.o

client: functions.o client.o 
	colorgcc -Wall client.o functions.o -o client

client.o: client.c
	colorgcc -Wall -c client.c -o client.o

server: functions.o server.o 
	colorgcc -Wall functions.o server.o -o server

server.o: server.c
	colorgcc -Wall -c server.c -o server.o

clean:
	rm *.o server client
