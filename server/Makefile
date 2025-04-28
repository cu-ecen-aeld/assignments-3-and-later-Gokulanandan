ifeq ($(CROSS_COMPILE), aarch64-none-linux-gnu-)
    CC = aarch64-none-linux-gnu-gcc
else
    CC = gcc
endif

all default:
	$(CC) $(CFLAGS) -o aesdsocket aesdsocket.c

clean:
	rm -f aesdsocket
