#!/bin/sh

set -xe

mkdir -p ./out

export PKG_CONFIG_PATH=$HOME/opt/SDL3/lib/pkgconfig

gcc -Wall -Werror -Wvla -std=c23 $(pkg-config --cflags sdl3) \
	-o out/main main.c $(pkg-config --libs sdl3)

./out/main &
