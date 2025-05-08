#pragma once

#include <stdio.h>

typedef unsigned char byte;

typedef struct {
    char **img_paths;
    int path_count;
    int width;
    int height;
    size_t target_size;
} params;

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
