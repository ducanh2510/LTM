all:
	clear
	gcc `pkg-config --cflags gtk+-3.0` -o main main.c `pkg-config --libs gtk+-3.0` -pthread
	gcc -o server server.c linked_list.h -pthread
s:
	clear
	./server 5500
c:
	clear
	./main