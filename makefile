all:
	clear
	$ export GDK_SYNCHRONIZE=1
	gcc `pkg-config --cflags gtk+-3.0` -o client client.c `pkg-config --libs gtk+-3.0` -pthread
	gcc -o server server.c linked_list.h -pthread
s:
	clear
	./server 9000
c:
	clear
	./client
clean:
	rm -f *.o *~