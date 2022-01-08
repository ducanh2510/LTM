all:
	clear
	gcc `pkg-config --cflags gtk+-3.0` -o client client.c linked_list.h `pkg-config --libs gtk+-3.0` -pthread
	gcc -o server server.c linked_list.h -pthread
s:
	clear
	./server 5500
c:
	clear
	./client
clean:
	rm -f *.o *~