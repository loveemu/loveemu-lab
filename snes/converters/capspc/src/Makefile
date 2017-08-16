# Makefile for capspc

CC	= cc
CFLAGS	= -O
LDFLAGS	=
INCLUDES = -I.
LIBS	= -lm
TARGET	= capspc
OBJS	= cioutil.o libsmfc.o libsmfcx.o spcseq.o capspc.o

all:	$(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	-rm -f $(TARGET) $(OBJS) .nfs* *~ \#* core

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

libsmfc.h: cioutil.h
libsmfcx.h: cioutil.h libsmfc.h
chunspc.h: cioutil.h libsmfc.h libsmfcx.h
cioutil.o: cioutil.h
libsmfc.o: libsmfc.h
libsmfcx.o: libsmfcx.h
spcseq.o: spcseq.h
capspc.o: capspc.h
