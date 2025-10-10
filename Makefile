# Define the output file name based on the OS
ifeq ($(shell uname), Darwin)
    CLIENT_EXE = client.pas
	SERVER_EXE = server.pas
else
    CLIENT_EXE = client
	SERVER_EXE = server
endif

OBJS = awale.o challenge.o game.o message.o utils.o user_interaction.o ranking.o persistence.o

all: client server

message.o: Server/message.c
	gcc -c Server/message.c 

game.o: Server/game.c
	gcc -c Server/game.c 

challenge.o: Server/challenge.c
	gcc -c Server/challenge.c 

utils.o: Server/utils.c
	gcc -c Server/utils.c 

user_interaction.o: Server/user_interaction.c
	gcc -c Server/user_interaction.c 

ranking.o: Server/ranking.c
	gcc -c Server/ranking.c 

persistence.o: Server/persistence.c
	gcc -c Server/persistence.c

awale.o: awale.c
	gcc -c awale.c 

client.o: Client/client.c
	gcc -c Client/client.c 

server.o: Server/server.c
	gcc -c Server/server.c 

client: client.o
	gcc -o $(CLIENT_EXE) client.o

server: server.o $(OBJS)
	gcc -o $(SERVER_EXE) server.o $(OBJS) -lm

clean:
	rm -f *.o $(CLIENT_EXE) $(SERVER_EXE)