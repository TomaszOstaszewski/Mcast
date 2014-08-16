.DEFAULT_GOAL:=all
.SECONDEXPANSION:
.PHONY	:= clean
#CROSS_COMPILE:=arm-davinci2-linux-gnueabi-
CC	:=$(CROSS_COMPILE)gcc
STRIP	:=$(CROSS_COMPILE)strip
CFLAGS	:=-Wall -Werror -O0 -ggdb -ffunction-sections -fdata-sections -flto
BUILD_ROOT :=$(shell $(CC) -dumpmachine)/

all: $(addprefix $(BUILD_ROOT), \
	mcast-sender \
	mcast-receiver \
	ut-mcast-join-leave \
)

SOURCES:=\
 circular-buffer-uint8.c \
 debug_helpers.c \
 mcast-receiver-linux.c\
 mcast-sender-linux.c\
 mcast-setup-linux.c\
 mcast_utils.c \
 mcast_setup.c \
 platform-sockets.c\
 resolve.c\
 ut-circular-buffer-uint8.c\
 ut-mcast-join-leave.c \

OBJECTS:=$(SOURCES:%.c=$(BUILD_ROOT)%.o)
DEPENDS:=$(SOURCES:%.c=$(BUILD_ROOT)%.d)

.PRECIOUS: %/.
%/.:
	mkdir -pv $(@)

-include $(DEPENDS)

$(BUILD_ROOT)ut-mcast-join-leave: $(addprefix $(BUILD_ROOT),ut-mcast-join-leave.o mcast_utils.o mcast_setup.o debug_helpers.o platform-sockets.o resolve.o)
	$(CC) $(CFLAGS) -o $(@) $(^)

$(BUILD_ROOT)ut-circular-buffer-uint8: $(addprefix $(BUILD_ROOT),ut-circular-buffer-uint8.o circular-buffer-uint8.o)
	$(CC) $(CFLAGS) -o $(@) $(^) -Wl,--gc-sections

$(BUILD_ROOT)mcast-sender: $(addprefix $(BUILD_ROOT),mcast-sender-linux.o mcast_utils.o mcast-setup-linux.o debug_helpers.o platform-sockets.o resolve.o)
	$(CC) $(CFLAGS) -o $(@) $(^) -Wl,--gc-sections

$(BUILD_ROOT)mcast-receiver: $(addprefix $(BUILD_ROOT),mcast-receiver-linux.o mcast_utils.o mcast-setup-linux.o debug_helpers.o platform-sockets.o resolve.o)
	$(CC) $(CFLAGS) -o $(@) $(^)

$(BUILD_ROOT)%.o: %.c | $$(@D)/.
	$(CC) -MF $(@D)/$(*).d -MMD $(CFLAGS) -c -o $@ $<

$(BUILD_ROOT)smcast-sender: udp-unicast-connmcast-sender
	$(STRIP) -o $(@) $(^)

.PHONY: send
send: $(BUILD_ROOT)smcast-sender $(BUILD_ROOT)ut-mcast-join-leave
	scp $(^) root@10.171.33.103:/mnt/flash/

.PHONY: ut
ut: ut-mcast-join-leave
	ssh root@10.171.33.103 /mnt/flash/ut-mcast-join-leave

cscope:
	@cscope -bk -I$(MINGWPSDKINCLUDE)

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(DEPENDS)
	$(RM) -r $(BUILD_ROOT)
