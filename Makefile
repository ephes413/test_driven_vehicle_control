CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

TARGET = vehicle_simulation
SRC = $(wildcard src/*.c)
HEADERS = $(wildcard include/*.h)

.PHONY: all run test test-unit test-integration test-requirement analyze-impact clean

.DEFAULT_GOAL := all

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

test: test-unit test-integration

test-unit:
	python3 test/test_unit.py

test-integration:
	python3 test/test_integration.py

REQ_ID := $(filter-out test-requirement,$(MAKECMDGOALS))

test-requirement:
	python3 test/test_requirement.py $(REQ_ID)

IMPACT_ID := $(filter-out analyze-impact,$(MAKECMDGOALS))

analyze-impact:
	python3 test/analyze_impact.py $(IMPACT_ID)

ifneq ($(filter test-requirement analyze-impact,$(MAKECMDGOALS)),)
$(filter-out test-requirement analyze-impact,$(MAKECMDGOALS)):
	@:
endif

clean:
	rm -f $(TARGET)