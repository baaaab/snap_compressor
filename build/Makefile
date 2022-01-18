TARGET=snap_compressor

BUILDDIR := $(shell pwd)

SRC_PATHS += ../src/snap_compressor
SRC_PATHS += ../src/maths

VPATH = $(shell find $(SRC_PATHS) -type d)
VPATH += $(BUILDDIR)
SOURCES := $(shell find $(SRC_PATHS) -name "*.cpp")
OBJECTS := $(addprefix $(BUILDDIR)/,$(notdir $(SOURCES:%.cpp=%.o)))

CC=g++
LD=g++
CFLAGS=-MMD -std=c++11 -Ofast -ffast-math -march=native # -ggdb -O0 -fno-inline
CFLAGS+=-I../src/maths
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

