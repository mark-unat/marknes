# Makefile for marknes

OUT := marknes

CPPFLAGS := -Wall
LDFLAGS := -lglut -lGL

SRCS := \
	src/Cartridge.cpp \
	src/CpuBus.cpp \
	src/Cpu.cpp \
	src/Mapper000.cpp \
	src/Memory2KB.cpp \
	src/Controller.cpp \
	src/NameTable.cpp \
	src/PaletteTable.cpp \
	src/PpuBus.cpp \
	src/Ppu.cpp \
	src/Nes.cpp \
	src/main.cpp \

OBJS := $(SRCS:.cpp=.o)

$(OUT): $(OBJS)
	$(CXX) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) -rf $(OUT) $(OBJS)

