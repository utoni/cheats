CC = i686-w64-mingw32-gcc
CXX = i686-w64-mingw32-g++
CFLAGS := -Iinclude -Os -s -Wall -fvisibility=hidden -ffunction-sections -fdata-sections -ffast-math -fomit-frame-pointer -fexpensive-optimizations -Wl,--gc-sections -m32 -static -static-libgcc -static-libstdc++
LDFLAGS :=

TARGETS := h1z1

H1Z1_SRC := main.c
ifneq ($(strip $(X64)),)
CFLAGS+=-Ilibghack/CSFML-2.1-windows-64bits/CSFML-2.1/include -Ilibghack/include
LDFLAGS+=-Llibghack/CSFML-2.1-windows-64bits/CSFML-2.1/lib/gcc -Llibghack/CSFML-2.1-windows-64bits/CSFML-2.1/bin -Llibghack
else
CFLAGS+=-Ilibghack/CSFML-2.1-windows-32bits/CSFML-2.1/include -Ilibghack/include
LDFLAGS+=-Llibghack/CSFML-2.1-windows-32bits/CSFML-2.1/lib/gcc -Llibghack/CSFML-2.1-windows-32bits/CSFML-2.1/bin -Llibghack
endif


all: $(TARGETS)

%.o: %.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	$(CC) $(CFLAGS) -D_GNU_SOURCE=1 -std=gnu99 $(CFLAGS) -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ $<
	@echo 'Finished building: $<'
	@echo ' '

libghack/libghack.dll:
	make -C libghack

h1z1: libghack/libghack.dll main.o
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	$(CXX) $(CFLAGS) $(LDFLAGS)  -o $@ $^ -lcsfml-window -lcsfml-system -lcsfml-graphics libghack/libghack.dll -lpsapi
	@echo 'Finished building target: $@'
	@echo ' '


clean:
	$(MAKE) -C libghack clean
	rm -f h1z1.exe *.o *.d
