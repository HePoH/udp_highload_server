CFLAGS = -g -O2 -Wall -pedantic -ansi
LFLAGS =

vpath %.h ../include/
vpath %.c ../lib/

server.o: server.c server_core.c
		gcc -c $< $(CFLAGS) $(LFLAGS)

client.o: client.c client_core.c
		gcc -c $< $(CFLAGS) $(LFLAGS)

benchmark.o: benchmark.c client_core.c
		gcc -c $< $(CFLAGS) $(LFLAGS)

clean:                                                                                 
		rm -rf *.o *.out
