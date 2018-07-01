CC = i686-w64-mingw32-gcc
CXX = i686-w64-mingw32-g++
CFLAGS := -Iinclude -Os -s -Wall -fvisibility=hidden -ffunction-sections -fdata-sections -ffast-math -fomit-frame-pointer -fexpensive-optimizations -Wl,--gc-sections -m32 -static -static-libgcc -static-libstdc++
LDFLAGS :=

TARGETS := aoe2hd

AOE2HD_SRC := CodeGenerator.cpp CodeInjector.cpp CodePatcher.cpp main.cpp ModuleMemory.cpp utils.cpp


all: $(TARGETS)

%.o: %.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	$(CC) $(CFLAGS) -D_GNU_SOURCE=1 -DPSAPI_VERSION=1 -std=gnu99 $(CFLAGS) -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ $<
	@echo 'Finished building: $<'
	@echo ' '
%.o: %.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	$(CXX) $(CFLAGS) -D_GNU_SOURCE=1 -DPSAPI_VERSION=1 -std=c++11 $(CFLAGS) -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ $<
	@echo 'Finished building: $<'
	@echo ' '


aoe2hd: $(patsubst %.cpp,src/%.o,$(AOE2HD_SRC)) src/native.o
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	$(CXX) $(CFLAGS) $(LDFLAGS)  -o $@ $^ -lpsapi
	@echo 'Finished building target: $@'
	@echo ' '
