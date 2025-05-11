#pragma once

#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
#else
    #include <sys/types.h>
#endif

#ifndef byte
typedef unsigned char byte;
#endif

typedef struct
{
    char src[PATH_MAX];
    char dest[PATH_MAX];
    char **img_paths;
    int path_count;
    int width;
    int height;
    ssize_t target_size;
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
