CROSS_TOOL = aarch64-linux-gnu-gcc
CC = $(CROSS_TOOL)
CFLAGS = -Wall -g -std=c99 -D_POSIX_C_SOURCE=200809L -Werror -Wshadow
LFLAGS = -L$(HOME)/cmpt433/public/v4l2_lib_BB -lv4l2
TARGET = capture_exec
SRC = capture.c
DEST_DIR = /home/justin/cmpt433/public/myApps/
all: $(TARGET)
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LFLAGS)
install: $(TARGET)
	cp $(TARGET) $(DEST_DIR)
clean:
	rm -f $(TARGET)
.PHONY: all install clean
