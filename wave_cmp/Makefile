TARGET=wave_cmp

BUILDDIR := $(shell pwd)

SRC_PATHS += ./
SRC_PATHS += ../src/display/
SRC_PATHS += ../src/fft/

VPATH = $(shell find $(SRC_PATHS) -type d)
VPATH += $(BUILDDIR)
SOURCES := $(shell find $(SRC_PATHS) -name "*.cpp")
OBJECTS := $(addprefix $(BUILDDIR)/,$(notdir $(SOURCES:%.cpp=%.o)))

CC=g++
LD=g++
CFLAGS=-MMD -std=c++11 -O2 -ffast-math -ggdb
CFLAGS+=-I../include
LDFLAGS= -lSDL2 -lX11
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

-include $(OBJECTS:.o=.d)

$(BUILDDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -I$(dir $<) -c $< -o $@
	
clean:
	rm -f $(TARGET)
	rm -rf *.o
	rm -rf *.d

