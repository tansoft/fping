AM_CFLAGS = -Wall -Wextra -Wno-sign-compare

ifndef __APPLE__
AM_LDFLAGS += -static
endif

AM_LDFLAGS += -lcurl

sbin_PROGRAMS = fping

fping_SOURCES = fping.c seqmap.c socket4.c fping.h options.h seqmap.h optparse.c optparse.h central_mode.inc
fping_DEPENDENCIES = ../config.h ../central_mode.h

if IPV6
fping_SOURCES += socket6.c
fping_CFLAGS = $(AM_CFLAGS) -DIPV6
endif
