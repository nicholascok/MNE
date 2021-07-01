CC			:= gcc
CC_FLAGS	:= -lm -O0

TARGET_DIR	:= build
HEADERS		:= bmap2.h
OBJS		:= bmap2.c

.PHONY: all compile build clean

all: build

build:
	@echo "building..."
	$(CC) -o $(TARGET_DIR)/bmap $(HEADERS) $(OBJS) $(CC_FLAGS)

clean:
	@echo "cleaning build directory..."
	rm -r ./build/*
