CC = i686-w64-mingw32-gcc
CXX = i686-w64-mingw32-g++
CFLAGS := -Iinclude -Os -s -Wall -fvisibility=hidden -ffunction-sections -fdata-sections -ffast-math -fomit-frame-pointer -fexpensive-optimizations -Wl,--gc-sections -m32 -static -static-libgcc -static-libstdc++
LDFLAGS :=

TARGETS := csgo_wh

CSGOWH_SRC := D9DW.cpp D9DW_Rectangle.cpp D9DW_Text.cpp DLLMain.cpp Game.cpp Hook.cpp


all: $(TARGETS)

%.o: %.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	$(CXX) $(CFLAGS) -D_GNU_SOURCE=1 -DPSAPI_VERSION=1 -std=c++11 $(CFLAGS) -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ $<
	@echo 'Finished building: $<'
	@echo ' '


csgo_wh: $(patsubst %.cpp,src/%.o,$(CSGOWH_SRC))
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	$(CXX) $(CFLAGS) $(LDFLAGS)  -shared -o $@.dll $^ -lpsapi -luser32 -ld3d9 -ld3dx9
	@echo 'Finished building target: $@'
	@echo ' '
