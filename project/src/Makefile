CC = gcc
CFLAGS = -O2

FMT = indent 

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)
FILES_TO_FORMAT = $(SOURCES) $(HEADERS)

OBJECTS = $(SOURCES:.c=.o)

all: mysh

mysh: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^
	
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -DFRAMESIZE=$(framesize) -DVARMEMSIZE=$(varmemsize) -c $< -o $@

style:
	$(FMT) $(FMT_OPTIONS) $(FILES_TO_FORMAT)

clean:
	rm -f mysh $(OBJECTS) *~ .*.swp # Remove executable, objects, backups, swap files

.PHONY: all style clean

