prefix=/usr/local
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
libdir=${exec_prefix}/lib
includedir=${prefix}/include
ARCH=X86
SYS=LINUX
CC=gcc
CFLAGS=-Wshadow -O3 -ffast-math  -Wall -I. -march=i686 -mfpmath=sse -msse -std=gnu99 -I/usr/X11R6/include -fomit-frame-pointer -fno-tree-vectorize
DEPMM=-MM -g0
DEPMT=-MT
LD=gcc -o 
LDFLAGS=-L/usr/X11R6/lib -lX11  -lm -lpthread
LIBX264=libx264.a
AR=ar rc 
RANLIB=ranlib
STRIP=strip
AS=yasm
ASFLAGS= -O2 -f elf -DBIT_DEPTH=8
EXE=
HAVE_GETOPT_LONG=1
DEVNULL=/dev/null
PROF_GEN_CC=-fprofile-generate
PROF_GEN_LD=-fprofile-generate
PROF_USE_CC=-fprofile-use
PROF_USE_LD=-fprofile-use
default: cli
install: install-cli
SOSUFFIX=so
SONAME=libx264.so.115
SOFLAGS=-shared -Wl,-soname,$(SONAME)
default: lib-shared
install: install-lib-shared
default: lib-static
install: install-lib-static
LDFLAGSCLI = 
CLI_LIBX264 = $(LIBX264)
