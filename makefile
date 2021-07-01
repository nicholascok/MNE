CC			:= gcc
CC_FLAGS	:= -O0 -lm

TARGET_DIR	:= build
HEADERS		:= 
OBJS		:= mne.c

.PHONY: all compile build clean

all: build

build:
	@echo "building..."
	$(CC) -o $(TARGET_DIR)/ray $(HEADERS) $(OBJS) $(CC_FLAGS)

clean:
	@echo "cleaning build directory..."
	rm -r ./build/*
