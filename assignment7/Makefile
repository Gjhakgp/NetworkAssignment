all: hello

hello: r_socket.o
	ar rs librsocket.a r_socket.o 	

rsocket.o: r_socket.c
	gcc -c  r_socket.c
	
