AM_CFLAGS = -Wall -Wextra -Wno-sign-compare

if DARWIN
AM_LDFLAGS =
AM_CFLAGS += -Wno-error=implicit-function-declaration
AM_CXXFLAGS = -Icrtlib -I/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1 -std=c++98
else	
AM_LDFLAGS = -static
AM_CXXFLAGS = -Icrtlib
endif

sbin_PROGRAMS = fping

fping_SOURCES = fping.c seqmap.c socket4.c fping.h options.h seqmap.h optparse.c optparse.h central_mode.inc central_func.cpp
fping_DEPENDENCIES = ../config.h central_func.h

if IPV6
fping_SOURCES += socket6.c
fping_CFLAGS = $(AM_CFLAGS) -DIPV6
endif
