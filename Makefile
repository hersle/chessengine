CFLAGS = -Wall

all: CFLAGS += -O3
all: chessengine

multithread: CFLAGS += -O3 -fopenmp
multithread: chessengine

debug: CFLAGS += -g
debug: chessengine

android: CC = arm-linux-gnueabi-gcc
android: CFLAGS += -O3 -static
android: chessengine

chessengine: chess.o move.o check.o score.o io.o utils.o

clean: 
	rm -f chessengine chess.o move.o check.o score.o io.o utils.o
