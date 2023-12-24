CC = gcc
CFLAGS=-Wall -Wextra
TARGET=SimpleCache

all:
	$(CC) $(CFLAGS) SimpleProgram.c L2Cache2W.c -o $(TARGET)

L1:
	$(CC) $(CFLAGS) SimpleProgram.c L1Cache.c -o $(TARGET)

L2-1:
	$(CC) $(CFLAGS) SimpleProgram.c L2Cache1W.c -o $(TARGET)

L2-2:
	$(CC) $(CFLAGS) SimpleProgram.c L2Cache2W.c -o $(TARGET)

clean:
	rm $(TARGET)