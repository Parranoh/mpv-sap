CC = gcc
LD = gcc
CFLAGS = -Wall -Wextra -Wpedantic -fPIC
asap = asap-5.1.0

mpv-sap.so:	asap.o mpv-sap.o
	$(LD) -shared -o $@ $^

asap.o:
	$(CC) $(CFLAGS) -c -o $@ $(asap)/asap.c

.PHONY: clean
clean:
	rm -rf asap.o mpv-sap.o mpv-sap.so
