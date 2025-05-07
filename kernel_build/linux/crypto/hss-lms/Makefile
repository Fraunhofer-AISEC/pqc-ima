# This Makefile can be used with GNU Make or BSD Make

#LIB=libfalcon-512_clean.a

#SOURCES = codec.c common.c fft.c fpr.c keygen.c pqclean.c rng.c sign.c vrfy.c
#OBJECTS = codec.o common.o fft.o fpr.o keygen.o pqclean.o rng.o sign.o vrfy.o
#HEADERS = api.h fpr.h inner.h

#CFLAGS=-std=c99 -O2 -W -Wall -Wconversion -Wextra -Wpedantic -Wvla -Werror -Wmissing-prototypes -Wredundant-decls -I../../../common $(EXTRAFLAGS)

#all: $(LIB)
#%.o: %.c $(HEADERS)
	#$(CC) $(CFLAGS) -c -o $@ $<
#$(LIB): $(OBJECTS)
	#$(AR) -r $@ $(OBJECTS)
#clean:
	#$(RM) $(OBJECTS)
	#$(RM) $(LIB)

include Kbuild

KERNELRELEASE ?= 6.8.2
KDIR ?= $(shell pwd)/../../../../linux

default: modules
install: modules_install

modules modules_install:
	$(MAKE) -C $(KDIR) M=$(PWD) $@

modules modules_install clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $@
