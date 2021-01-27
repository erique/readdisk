#!/bin/bash

# Assumes 'm68k-amigaos-gcc' in the $PATH, and $VBCC pointing to the bebbo/amiga-gcc installation root

set -ex

CC=m68k-amigaos-gcc
CFLAGS="-mcrt=nix -fdata-sections -m68020 -Os -Werror -Wall -Wno-pointer-sign -fomit-frame-pointer -ffreestanding"
LDFLAGS="-Wl,--gc-sections -Wl,--fatal-warnings -Wl,--no-undefined"
LIBS="$VBCC/m68k-amigaos/ndk/lib/linker_libs/debug.lib $VBCC/m68k-amigaos/ndk/lib/linker_libs/small.lib"
SRCS="readdisk.c"
EXE="readdisk.exe"

$CC $SRCS -o $EXE $CFLAGS $LDFLAGS $LIBS
