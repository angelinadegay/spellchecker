# Define compiler
CC=gcc

# Compiler flags
CFLAGS=-Wall -g

# Target executable name
TARGET=spchk

# Source files
SOURCES=spchk.c

# Object files
OBJECTS=$(SOURCES:.c=.o)

# Rule to make the target
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

# Rule to make object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(TARGET) $(OBJECTS)

# Phony targets
.PHONY: clean



