# Makefile per il tunnel

HEADERS = header

# Compilazione/Linking
GCC = gcc
GCCFLAGS = -Wall -Wunused -ansi -pedantic -pg 
LINKERFLAGS = -lm -lpthread


# Obiettivi
all: mit tunnelTX tunnelRX ric 

util.o: util.c
	$(GCC) -c $(GCCFLAGS)  -I $(HEADERS) util.c

mit: mit.o util.o
	$(GCC) -o mit ${GCCFLAGS}  mit.o  util.o $(LINKERFLAGS)

mit.o: mit.c
	$(GCC) -c $(GCCFLAGS) -I $(HEADERS) mit.c

tunnelTX: tunnelTX.o util.o
	$(GCC) -o tunnelTX ${GCCFLAGS}  tunnelTX.o  util.o $(LINKERFLAGS)

tunnelTX.o: tunnelTX.c
	$(GCC) -c $(GCCFLAGS) -I $(HEADERS) tunnelTX.c

tunnelRX: tunnelRX.o  util.o
	$(GCC) -o tunnelRX ${GCCFLAGS} tunnelRX.o  util.o ${LINKERFLAGS} 
	
tunnelRX.o: tunnelRX.c
	$(GCC) -c $(GCCFLAGS) -I $(HEADERS) tunnelRX.c

ric: ric.o util.o
	$(GCC) -o ric ${GCCFLAGS} ric.o  util.o ${LINKERFLAGS}

ric.o: ric.c
	$(GCC) -c $(GCCFLAGS)  -I $(HEADERS) ric.c

# Pulizia

clean:	
	rm -f *.o
	rm -f mit
	rm -f ric
	rm -f tunnelTX
	rm -f tunnelRX
	rm -f util
