TARGET=snap_compressor

BUILDDIR := $(shell pwd)

SRC_PATHS += ../src/snap_compressor
SRC_PATHS += ../src/maths

VPATH = $(shell find $(SRC_PATHS) -type d)
VPATH += $(BUILDDIR)
SOURCES_CPP := $(shell find $(SRC_PATHS) -name "*.cpp")
OBJECTS_CPP := $(addprefix $(BUILDDIR)/,$(notdir $(SOURCES_CPP:%.cpp=%.o)))
SOURCES_C := $(shell find $(SRC_PATHS) -name "*.c")
OBJECTS_C := $(addprefix $(BUILDDIR)/,$(notdir $(SOURCES_C:%.c=%.o)))

OBJECTS := $(OBJECTS_CPP) $(OBJECTS_C)

CC=g++
LD=g++
CFLAGS=-MMD -Ofast -ffast-math -march=native -ggdb # -O0 -fno-inline
CFLAGS+=-I../src/maths
CPPFLAGS=-std=c++11 
LDFLAGS=-llzma
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

-include $(OBJECTS:.o=.d)

$(BUILDDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) -I$(dir $<) -c $< -o $@
	
$(BUILDDIR)/%.o: %.c
	gcc $(CFLAGS) -I$(dir $<) -c $< -o $@
	
clean:
	rm -f $(TARGET)
	rm -rf *.o
	rm -rf *.d

