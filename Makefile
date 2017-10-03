CC=g++
COMPRESS=upx
COMPRESS_FLAG=-9 -q
OUT=strip.out
CFLAGS=-lrt -Wno-write-strings -Ofast -Wl,--gc-sections -fdata-sections -ffunction-sections -Wl,--strip-all

all: ws2812-rpi.o test.cpp bmp_read.h
	$(CC) ws2812-rpi.o test.cpp $(CFLAGS) -o $(OUT)
	$(COMPRESS) $(OUT) $(COMPRESS_FLAG)

ws2812-rpi.o: ws2812-rpi.h ws2812-rpi.cpp
	$(CC) -c ws2812-rpi.cpp $(CFLAGS)

clean:
	rm $(OUT) *.o
