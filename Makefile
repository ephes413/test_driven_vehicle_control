
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

TARGET = vehicle_simulation
SRC = $(wildcard src/*.c)
HEADERS = $(wildcard include/*.h)

.PHONY: all run test test-unit test-driving test-control test-requirement analyze-impact check-traceability clean

.DEFAULT_GOAL := all

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

# Run all tests
test: test-unit test-driving test-control

# Unit tests
UNIT_FUNC := $(if $(filter test-unit,$(MAKECMDGOALS)),$(filter-out test-unit,$(MAKECMDGOALS)))

test-unit:
	python3 test/test_unit.py $(UNIT_FUNC)

# Driving tests
test-driving:
	python3 test/test_driving.py

# Control tests
test-control:
	python3 test/test_control.py

# Requirement-based tests
REQ_ID := $(if $(filter test-requirement,$(MAKECMDGOALS)),$(filter-out test-requirement,$(MAKECMDGOALS)))

test-requirement:
	python3 test/test_requirement.py $(REQ_ID)

# Requirement impact analysis
IMPACT_ID := $(if $(filter analyze-impact,$(MAKECMDGOALS)),$(filter-out analyze-impact,$(MAKECMDGOALS)))

analyze-impact:
	python3 test/analyze_impact.py $(IMPACT_ID)

# Treat command-line parameters as valid Make targets
PARAM_TARGETS := $(filter test-unit test-requirement analyze-impact,$(MAKECMDGOALS))
PARAM_ARGS := $(filter-out test-unit test-requirement analyze-impact,$(MAKECMDGOALS))

ifneq ($(PARAM_TARGETS),)
$(PARAM_ARGS):
	@:
endif

# Traceability validation
check-traceability:
	python3 test/check_traceability.py

clean:
	rm -f $(TARGET)
