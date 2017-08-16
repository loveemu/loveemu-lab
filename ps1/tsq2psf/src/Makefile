#!/bin/make
CC = gcc
CFLAGS = -O2 -DHAVE_STDINT_H
CXX = g++
CXXFLAGS = -O2 -DHAVE_STDINT_H
LDFLAGS = -lm -lz
TARGET = tsq2psf
SRCS = $(TARGET).cpp PSFFile.cpp ZlibReader.cpp ZlibWriter.cpp
OBJS := $(SRCS:.cpp=.o)

.PHONY: all clean
.SUFFIXES: .c .cpp .o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $^
.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $^
