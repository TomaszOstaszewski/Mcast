.DEFAULT_GOAL:=all
.PHONY	:= clean
MINGWPSDKINCLUDE	:=/usr/i586-mingw32msvc/include/
CROSS_COMPILE:=i586-mingw32msvc-gcc
CFLAGS 	:=-Wall -Werror -ggdb -O0

all: mcast-sender

ut-circular-buffer-uint8: ut-circular-buffer-uint8.o circular-buffer-uint8.o	

mcast-sender: mcast-sender-linux.o mcast_utils.o mcast-setup-linux.o debug_helpers.o platform-sockets.o resolve.o
	$(CC) -o $(@) $(^)

%.o: %.c
	$(CC) -c -o $@ $<

cscope:
	@cscope -bk -I$(MINGWPSDKINCLUDE)
clean:
	$(RM) ex-perf-counter \
 ex-perf-counter.o \
 ut-circular-buffer-uint8 \
 ut-circular-buffer-uint8.exe \
 ut-circular-buffer-uint8.o \
 circular-buffer-uint8.o \
 mcast-setup-linux.o \
 mcast-sender-linux.o \
 debug_helpers.o \
 platform-sockets.o \
 resolve.o \
 mcast-sender \
 mcast_utils.o 
