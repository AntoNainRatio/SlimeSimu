CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0 epoxy` -Wall -O3 -march=native -ffast-math
LDFLAGS = `pkg-config --libs gtk+-3.0 epoxy` -lm
TARGET = simu
SRCS = main.c ship.c phero.c simu.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean