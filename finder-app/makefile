ifdef CROSS_COMPILE
	ifeq ($(CROSS_COMPILE), $(strip aarch64-none-linux-gnu-))
		override CC=aarch64-none-linux-gnu-gcc
	endif
endif

TARGET=writer

# aarch64-none-linux-gnu-gcc

all: writer.c
	@echo Make $@ ...
	$(CC) -g -o $(TARGET) writer.c

.PHONY: clean
clean:
	@echo "Clean build-products."
	@rm $(TARGET)