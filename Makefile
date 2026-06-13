CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0 epoxy` -Wall -O3 -march=native -ffast-math
LDFLAGS = `pkg-config --libs gtk+-3.0 epoxy` -lm
TARGET = simu
SRCS = src/main.c src/ship.c src/phero.c src/simu.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
