CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
LIBS   = -lz
SRC    = main.c pack.c unpack.c
OBJ    = $(SRC:.c=.o)
TARGET = sar
PREFIX = /usr/local/bin

all: $(TARGET) clean

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

%.o: %.c sar.h
	$(CC) $(CFLAGS) $(LIBS) -c -o $@ $<

install: $(TARGET)
	@echo "installing $(TARGET) to $(PREFIX)/$(TARGET)"
	install -m 755 $(TARGET) $(PREFIX)/$(TARGET)

uninstall:
	@echo "removing $(PREFIX)/$(TARGET)"
	rm -f $(PREFIX)/$(TARGET)

clean:
	rm -f $(OBJ)

veryclean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
