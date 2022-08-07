CC = g++
RM = rm
main: main.cpp
	@$(CC) -o client main.cpp
	@$(CC) -o server server.cpp