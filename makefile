CC = gcc
CFLAGS = -Wall -Wextra -g

# List your source files here
SOURCES = rush.c

# Name for your executable
EXECUTABLE = rush

all: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE)

.PHONY: clean
clean:
	rm -f $(EXECUTABLE)
