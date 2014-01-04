#!/bin/make
CC = gcc
CFLAGS = -O2 -Wall
CXX = g++
CXXFLAGS = -O2 -Wall
LDFLAGS = -lm
OBJS = seqq2mid

.PHONY: all clean
.SUFFIXES: .c .cpp .o

all: $(OBJS)

clean:
	$(RM) $(OBJS)

$(OBJS): seqq2mid.o ../common/cutils.o

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $^
.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $^
.c:
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
.cpp:
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
