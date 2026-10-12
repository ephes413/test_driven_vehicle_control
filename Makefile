CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

TARGET = vehicle_simulation
SRC = $(wildcard src/*.c)
HEADERS = $(wildcard include/*.h)

.PHONY: all run test test-unit test-integration test-requirement clean

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

test: test-unit test-integration

test-unit:
	python3 test/test_driver.py

test-integration:
	python3 test/test_integration.py

REQ_ID := $(filter-out test-requirement,$(MAKECMDGOALS))

test-requirement:
	python3 test/test_requirement.py $(REQ_ID)

ifneq ($(filter test-requirement,$(MAKECMDGOALS)),)
$(REQ_ID):
	@:
endif

clean:
	rm -f $(TARGET)
