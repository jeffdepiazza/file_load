CFLAGS = -v -Wall -Wextra -g3
BINARY_NAME = file_load

default: all

clean:
	rm -rf *.o *.s $(BINARY_NAME)

all: $(BINARY_NAME)

$(BINARY_NAME): $(BINARY_NAME).o
	gcc $(CFLAGS) $(BINARY_NAME).o -o $(BINARY_NAME)

$(BINARY_NAME).o: $(BINARY_NAME).c
	gcc $(CFLAGS) -c $(BINARY_NAME).c -o $(BINARY_NAME).o