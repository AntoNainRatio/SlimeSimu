CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0` -Wall -O3
LDFLAGS = `pkg-config --libs gtk+-3.0` -lm
TARGET = simu
SRCS = main.c ship.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean

