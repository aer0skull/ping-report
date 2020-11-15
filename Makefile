CC=gcc
CFLAGS=-W -Wall -pedantic
LDFLAGS=-lsqlite3
EXEC=ping-report
SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)

all : $(EXEC)

ping-report : $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)
	rm -f src/*.o

ping-report.o : include/daemon.h

daemon.o : include/stats.h include/utils.h

stats.o : include/utils.h

%.o : src/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean :
	rm -f src/*.o

mrproper : clean
	rm -f ping-report
