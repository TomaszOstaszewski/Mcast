.DEFAULT_GOAL:=all
.PHONY	:= clean
CROSS_COMPILE:=arm-davinci2-linux-gnueabi-
CC	:=$(CROSS_COMPILE)gcc
STRIP	:=$(CROSS_COMPILE)strip
CFLAGS	:=-Wall -Werror -O3 -ggdb -ffunction-sections -fdata-sections -flto

all: mcast-sender mcast-receiver

SOURCES:=\
 circular-buffer-uint8.c \
 debug_helpers.c \
 mcast-receiver-linux.c\
 mcast-sender-linux.c\
 mcast-setup-linux.c\
 mcast_utils.c\
 platform-sockets.c\
 resolve.c\
 ut-circular-buffer-uint8.c\
 ut-mcast-join-leave.c \

OBJECTS:=$(SOURCES:%.c=%.o)
DEPENDS:=$(SOURCES:%.c=%.d)

-include $(DEPENDS)

ut-mcast-join-leave: ut-mcast-join-leave.o mcast_utils.o debug_helpers.o platform-sockets.o resolve.o
	$(CC) $(CFLAGS) -o $(@) $(^)

ut-circular-buffer-uint8: ut-circular-buffer-uint8.o circular-buffer-uint8.o	

mcast-sender: mcast-sender-linux.o mcast_utils.o mcast-setup-linux.o debug_helpers.o platform-sockets.o resolve.o
	$(CC) $(CFLAGS) -o $(@) $(^) -Wl,--gc-sections

mcast-receiver: mcast-receiver-linux.o mcast_utils.o mcast-setup-linux.o debug_helpers.o platform-sockets.o resolve.o
	$(CC) $(CFLAGS) -o $(@) $(^)

%.o: %.c
	$(CC) -MF $(*).d -MMD $(CFLAGS) -c -o $@ $<

smcast-sender: mcast-sender
	$(STRIP) -o $(@) $(^)

.PHONY: send
send: smcast-sender ut-mcast-join-leave
	scp $(^) root@10.171.33.103:/mnt/flash/

.PHONY: ut
ut: ut-mcast-join-leave
	ssh root@10.171.33.103 /mnt/flash/ut-mcast-join-leave

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
 mcast-receiver-linux.o \
 debug_helpers.o \
 platform-sockets.o \
 resolve.o \
 mcast-sender \
 mcast_utils.o 
