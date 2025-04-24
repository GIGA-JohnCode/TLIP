#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <turbojpeg.h>

#include "types.h"

rgb* load_jpeg(char *jpeg_path);

bool store_jpeg(rgb* palette, size_t target_size, char* default_path);

