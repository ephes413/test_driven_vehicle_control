
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

TARGET = vehicle_simulation
SRC = $(wildcard src/*.c)
HEADERS = $(wildcard include/*.h)

.PHONY: all run test test-unit test-control test-requirement analyze-impact check-traceability clean

.DEFAULT_GOAL := all

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

test: test-unit test-control

UNIT_FUNC := $(filter-out test-unit,$(MAKECMDGOALS))

test-unit:
	python3 test/test_unit.py $(UNIT_FUNC)

test-control:
	python3 test/test_control.py

REQ_ID := $(filter-out test-requirement,$(MAKECMDGOALS))

test-requirement:
	python3 test/test_requirement.py $(REQ_ID)

IMPACT_ID := $(filter-out analyze-impact,$(MAKECMDGOALS))

analyze-impact:
	python3 test/analyze_impact.py $(IMPACT_ID)

PARAM_TARGETS := $(filter test-unit test-requirement analyze-impact,$(MAKECMDGOALS))
PARAM_ARGS := $(filter-out test-unit test-requirement analyze-impact,$(MAKECMDGOALS))

ifneq ($(PARAM_TARGETS),)
$(PARAM_ARGS):
	@:
endif

check-traceability:
	python3 test/check_traceability.py

clean:
	rm -f $(TARGET)
