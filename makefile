compile:
	gcc -pthread local_server.c -o local_server.out

test:
	gcc -pthread -g -D BULK local_server.c -o local_server.out
	
debug:
	gcc -pthread -g -D DEBUG local_server.c -o local_server.out

info:
	gcc -pthread -g -D INFO local_server.c -o local_server.out