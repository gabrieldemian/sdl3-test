#!/bin/sh

set -xe

mkdir -p out

export PKG_CONFIG_PATH=$HOME/opt/SDL3/lib/pkgconfig

glslc ./shader.glsl.vert -o shader.spv.vert
glslc ./shader.glsl.frag -o shader.spv.frag

gcc -Wall -Werror -Wvla -std=c23 -g $(pkg-config --cflags sdl3) \
	-o ./out/main main.c $(pkg-config --libs sdl3)
