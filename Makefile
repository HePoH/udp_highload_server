IDIR = include
LDIR = lib
SDIR = src
LFLAGS = -lpthread
CFLAGS = -g -O2 -Wall -pedantic -ansi
CC = gcc

SUBDIRS = $(LDIR) $(SDIR)
OBJSERV = $(SDIR)/server.o $(LDIR)/server_core.o
OBJCLIENT = $(SDIR)/client.o $(LDIR)/client_core.o
OBJBENCHMARK = $(SDIR)/benchmark.o $(LDIR)/client_core.o

vpath %.c $(SDIR) : $(LDIR)
vpath %.o $(SDIR) : $(LDIR)
vpath %.h $(IDIR)

all: server client benchmark

server: $(OBJSERV)
		$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS) 

$(OBJSERV): prepare

client: $(OBJCLIENT)
		$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS) 

$(OBJCLIENT): prepare

benchmark: $(OBJBENCHMARK)
		$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS) 

$(OBJBENCHMARK): prepare

.PHONY: prepare $(SUBDIRS)
prepare: $(SUBDIRS)

$(SUBDIRS):
		make -C $@
		
$(SDIR): $(LDIR)

clean:
		clear
		cd src	&& $(MAKE) clean
		cd lib	&& $(MAKE) clean
		rm -rf server client benchmark *.o
