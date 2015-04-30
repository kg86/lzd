# Makefile

SRCS	= txt2cfg_online.c txt2cfg_offline.c cfg2txt.c cfg2enc.c enc2txt.c bits.c
LCACOMP = lcacomp
OFF = lcacomp_offline
TEST	= txt2cfg_online txt2cfg_offline cfg2txt cfg2enc enc2txt txt2enc_online txt2enc_offline

OBJS	= $(SRCS:%.c=%.o)
CC	= /opt/local/bin/gcc-mp-4.9
CFLAGS	= -Wall -Ofast -DNDEBUG
LIB	= -lm

all: $(LCACOMP)

off: $(OFF)

test: $(TEST)

lcacomp: lcacomp.c txt2cfg_online.o cfg2enc.o enc2txt.o bits.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIB)

lcacomp_offline: lcacomp_offline.c txt2cfg_online.o cfg2enc.o enc2txt.o bits.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIB)

txt2cfg_online: test_main.c txt2cfg_online.o
	$(CC) $(CFLAGS) -D_TXT2CFG_ONLINE -o $@ $^ $(LIB)

txt2cfg_offline: test_main.c txt2cfg_offline.o
	$(CC) $(CFLAGS) -D_TXT2CFG_OFFLINE -o $@ $^ $(LIB)

cfg2txt: cfg2txt.o
	$(CC) $(CFLAGS) -o $@ $^

cfg2enc: test_main.c cfg2enc.o bits.o
	$(CC) $(CFLAGS) -D_CFG2ENC -o $@ $^

enc2txt: test_main.c enc2txt.o bits.o bits.o
	$(CC) $(CFLAGS) -D_ENC2TXT -o $@ $^

txt2enc_online: test_main.c txt2cfg_online.o cfg2enc.o bits.o
	$(CC) $(CFLAGS) -D_TXT2ENC_ONLINE -o $@ $^ $(LIB)

txt2enc_offline: test_main.c txt2cfg_offline.o cfg2enc.o bits.o
	$(CC) $(CFLAGS) -D_TXT2ENC_OFFLINE -o $@ $^ $(LIB)

clean:
	-rm -f $(LCACOMP) $(TEST) $(OBJS)  *~

.c.o:
	$(CC) $(CFLAGS) -c $<

txt2cfg_online.o: txt2cfg_online.h lcacommon.h
txt2cfg_offline.o: txt2cfg_offline.h lcacommon.h
cfg2txt.o: lcacommon.h
cfg2enc.o: cfg2enc.h bits.h lcacommon.h
enc2txt.o: enc2txt.h bits.h lcacommon.h
bits.o: bits.h lcacommon.h
