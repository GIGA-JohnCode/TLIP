#pragma once

#include <stdio.h>

typedef unsigned char byte;

typedef struct
{
    byte *buffer;
    size_t size;
    int width;
    int height;
    int subsamp;
    int colorspace;
} jpeg;

typedef struct
{
    byte *buffer;
    size_t original_size;
    int width;
    int height;
    int components;
} rgb;
