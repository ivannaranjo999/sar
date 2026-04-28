CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
LIBS   = -lz
SRC    = main.c pack.c unpack.c
OBJ    = $(SRC:.c=.o)
TARGET = sar
PREFIX = /usr/local/bin

all: $(TARGET) clean

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c sar.h
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBS)

install: $(TARGET)
	install -m 755 $(TARGET) $(PREFIX)/$(TARGET)

uninstall:
	rm -f $(PREFIX)/$(TARGET)

clean:
	rm -f $(OBJ)

veryclean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
