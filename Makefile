CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lglfw -lGL -lm -lpthread -ldl
TARGET = main
SRC = main.c ship.c

# Rules
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
