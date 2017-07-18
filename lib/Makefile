CFLAGS = -g -O2 -Wall -pedantic -ansi
LFLAGS =

vpath %.h ../include/

server_core.o: server_core.c server_core.h
		gcc -c $< $(CFLAGS) $(LFLAGS)

client_core.o: client_core.c client_core.h
		gcc -c $< $(CFLAGS) $(LFLAGS)

clean:
		rm -rf *.o
