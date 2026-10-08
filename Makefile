CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

TARGET = vehicle_simulation
SRC = $(wildcard src/*.c)
HEADERS = $(wildcard include/*.h)

.PHONY: all run test clean

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

test:
	python3 test/test_driver.py

clean:
	rm -f $(TARGET)
	