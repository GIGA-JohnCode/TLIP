#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <turbojpeg.h>

#include "types.h"

rgb* load_jpeg(char *jpeg_path);
jpeg* read_jpeg(char *jpeg_path);
bool parse_jpeg(jpeg* image, tjhandle decompressor);
rgb* extract_rgb(jpeg* image, tjhandle decompressor);

bool store_jpeg(rgb* palette, size_t target_size, char* default_path);
bool encode_jpeg(rgb* palette, int quality, unsigned char **jpeg_buffer, unsigned long *jpeg_size);
bool write_jpeg(byte* jpeg_buffer, unsigned long jpeg_size, char* default_path);
