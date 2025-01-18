CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lpcap

TARGET = beacon-flooding-hw
SRC = beacon-flooding-hw.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f $(TARGET)

