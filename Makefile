CC = g++
CFLAGS = -Wall -g
TARGET = target/pimsys_linux
SRC = src/main.cpp linux/Hardware_Linux.cpp
INCLUDE = -Isrc -Ilinux

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p target
	$(CC) $(CFLAGS) $(INCLUDE) $(SRC) -o $(TARGET)

clean:
	rm -rf target

run: all
	./$(TARGET)
