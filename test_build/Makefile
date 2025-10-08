CC = gcc
CFLAGS = -Wall -g
TARGET = hello

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

clean:
	rm -f $(TARGET)
