CC = gcc
CFLAGS = -Wall -g
TARGET = cpe351
OBJS = cpe351.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

cpe351.o: cpe351.c cmpe351.h
	$(CC) $(CFLAGS) -c cpe351.c

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET) -t 2 -f input.txt -o output.txt
