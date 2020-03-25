CC = i686-w64-mingw32-gcc
CXX = i686-w64-mingw32-g++
CFLAGS := -Iinclude -Os -s -Wall -fvisibility=hidden -ffunction-sections -fdata-sections -ffast-math -fomit-frame-pointer -fexpensive-optimizations -Wl,--gc-sections -m32 -static -static-libgcc -static-libstdc++
LDFLAGS :=


all: csgo_radar

%.o: %.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	$(CC) $(CFLAGS) -D_GNU_SOURCE=1 -DPSAPI_VERSION=1 -DUNICODE=1 -std=gnu99 $(CFLAGS) -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ $<
	@echo 'Finished building: $<'
	@echo ' '


csgo_radar: main.o
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	$(CC) $(CFLAGS) $(LDFLAGS)  -o $@ $^ -lpsapi
	@echo 'Finished building target: $@'
	@echo ' '


clean:
	rm -f csgo_radar.exe *.o *.d
