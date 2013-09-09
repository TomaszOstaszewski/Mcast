.DEFAULT_GOAL:=all
.PHONY	:= clean
MINGWPSDKINCLUDE	:=/usr/i586-mingw32msvc/include/
CROSS_COMPILE:=i586-mingw32msvc-gcc
CFLAGS 	:=-Wall -Werror
all: mcast-setup-linux.o mcast_utils.o

ut-circular-buffer-uint8: ut-circular-buffer-uint8.o circular-buffer-uint8.o	

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
 mcast_utils.o 
