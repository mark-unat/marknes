# Makefile for marknes

OUT := marknes

CPPFLAGS := -Wall -std=c++14

# To add sidebar in our window
CPPFLAGS += -Ires/ -DSIDEBAR

LDFLAGS := -lglut -lGL -lopenal -lpthread

SRCS := \
	src/AudioHw.cpp \
	src/Apu.cpp \
	src/Cartridge.cpp \
	src/CpuBus.cpp \
	src/Cpu.cpp \
	src/Mapper000.cpp \
	src/Mapper002.cpp \
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

