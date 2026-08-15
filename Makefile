CC = gcc

TARGET = fmp

SRCDIR = ./src

CFLAGS = -o $(TARGET) -lmpv -O3

PREFIX = /usr

MACOS_PREFIX = /usr/local

all: $(TARGET)


install_mac:
	$(CC) $(SRCDIR)/main.c $(CFLAGS)
	install -m 755 $(TARGET) $(MACOS_PREFIX)/bin

install: 
	$(CC) $(SRCDIR)/main.c $(CFLAGS)
	install -m 755 $(TARGET) $(PREFIX)/bin

uninstall: 
	rm $(PREFIX)/bin/$(TARGET)

clean:
	rm -f ./$(TARGET)
