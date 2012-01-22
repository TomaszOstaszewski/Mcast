.DEFAULT_GOAL:=all
.PHONY	:= clean
MINGWPSDKINCLUDE	:=/usr/i586-mingw32msvc/include/
CC	:=i586-mingw32msvc-gcc
CFLAGS 	:=-Wall -Werror
all: ex-perf-counter
ex-perf-counter : perf-counter-itf.o	
cscope:
	@cscope -bk -I$(MINGWPSDKINCLUDE)
clean:
	$(RM) -f ex-perf-counter ex-perf-counter.o
