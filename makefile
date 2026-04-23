CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
LIBS   = -lz
SRC    = main.c pack.c unpack.c
OBJ    = $(SRC:.c=.o)
TARGET = sar

all: $(TARGET) clean

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

%.o: %.c sar.h
	$(CC) $(CFLAGS) $(LIBS) -c -o $@ $<

clean:
	rm -f $(OBJ)

veryclean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
