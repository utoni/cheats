CC = i686-w64-mingw32-gcc
CXX = i686-w64-mingw32-g++
CFLAGS := -Iinclude -Os -s -Wall -fvisibility=hidden -ffunction-sections -fdata-sections -ffast-math -fomit-frame-pointer -fexpensive-optimizations -Wl,--gc-sections -m32 -static -static-libgcc -static-libstdc++
LDFLAGS :=

TARGETS := ghack

GHACK_SRC := log.c mem.c radar.c
ifneq ($(strip $(X64)),)
CFLAGS+=-ICSFML-2.1-windows-64bits/CSFML-2.1/include
LDFLAGS+=-LCSFML-2.1-windows-64bits/CSFML-2.1/lib/gcc -LCSFML-2.1-windows-64bits/CSFML-2.1/bin
else
CFLAGS+=-ICSFML-2.1-windows-32bits/CSFML-2.1/include
LDFLAGS+=-LCSFML-2.1-windows-32bits/CSFML-2.1/lib/gcc -LCSFML-2.1-windows-32bits/CSFML-2.1/bin
endif


all: $(TARGETS)

%.o: %.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	$(CC) $(CFLAGS) -D_GNU_SOURCE=1 -std=gnu99 $(CFLAGS) -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ $<
	@echo 'Finished building: $<'
	@echo ' '


ghack: $(patsubst %.c,src/%.o,$(GHACK_SRC))
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	$(CXX) $(CFLAGS) $(LDFLAGS)  -shared -o libghack.dll $^ -lcsfml-window -lcsfml-system -lcsfml-graphics
	@echo 'Finished building target: $@'
	@echo ' '
