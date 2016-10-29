all: bank.o server client

bank.o: bank.c
	gcc -std=c99 -W -g -pthread -c  bank.c

server: server.c bank.o
	gcc -g -pthread -o server server.c bank.o

client: client.c
	gcc -W  -g -pthread -o client client.c

clean:
	rm  bank.o
	rm server
	rm client

