ifeq ($(CROSS_COMPILE), aarch64-none-linux-gnu-)
	CC = aarch64-none-linux-gnu-gcc
else
	CC = gcc
endif

writer all:
	$(CC) -o writer writer.c

clean:
	rm -f writer
