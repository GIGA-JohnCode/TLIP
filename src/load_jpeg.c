#include "jpeg.h"
#include "util.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <turbojpeg.h>

static jpeg* read_jpeg(const char *input_path);
static bool parse_jpeg(jpeg* image, tjhandle decompressor);
static rgb* extract_rgb(jpeg* image, tjhandle decompressor);

rgb* load_jpeg(const char *input_path)
{
    jpeg* image = read_jpeg(input_path);
    if (!image)
        return NULL;

    tjhandle decompressor = tjInitDecompress();
    if (!decompressor)
    {
        alert("ERROR", tjGetErrorStr2(NULL));
        free(image->buffer);
        free(image);
        return NULL;
    }

    if (!parse_jpeg(image, decompressor))
    {
        free(image->buffer);
        free(image);
        tjDestroy(decompressor);
        return NULL;
    }

    rgb* palette = extract_rgb(image, decompressor);

    free(image->buffer);
    free(image);
    tjDestroy(decompressor);

    if (!palette)
        return NULL;
    else
        return palette;
}

static jpeg* read_jpeg(const char *input_path)
{
    jpeg *image = (jpeg*)malloc(sizeof(jpeg));
    if (!image)
    {
        alert("ERROR", "Memory allocation failed.");
        return NULL;
    }

    FILE *fptr = fopen(input_path, "rb");
    if (!fptr)
    {
        alert("ERROR", "Failed to open image at given path.");
        free(image);
        return NULL;
    }

    fseek(fptr, 0, SEEK_END);
    image->size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    image->buffer = (byte*)malloc(sizeof(byte) * image->size);
    if (!image->buffer)
    {
        alert("ERROR", "Memory allocation failed.");
        free(image);
        fclose(fptr);
        return NULL;
    }

    if (fread(image->buffer, 1, image->size, fptr) != image->size)
    {
        alert("ERROR", "Failed to read image at given path.");
        free(image);
        fclose(fptr);
        return NULL;
    }

    fclose(fptr);
    return image;
}

static bool parse_jpeg(jpeg* image, tjhandle decompressor)
{
    if (tjDecompressHeader3(decompressor, image->buffer, image->size,
       &(image->width), &(image->height), &(image->subsamp), &(image->colorspace)) < 0)
    {
        alert("ERROR", tjGetErrorStr2(decompressor));
        return false;
    }
    return true;
}

static rgb* extract_rgb(jpeg* image, tjhandle decompressor)
{
    rgb* palette = (rgb*)malloc(sizeof(rgb));
    if (!palette)
    {
        alert("ERROR", "Memory allocation failed.");
        return NULL;
    }

    palette->original_size = image->size;
    palette->width = image->width;
    palette->height = image->height;
    palette->components = 3;

    palette->buffer = (byte*)malloc(palette->width * palette->height * palette->components);
    if (!palette->buffer)
    {
        alert("ERROR", "Memory allocation failed.");
        free(palette);
        return NULL;
    }

    if (tjDecompress2(decompressor, image->buffer, image->size,
        palette->buffer, palette->width, 0, palette->height, TJPF_RGB, TJFLAG_ACCURATEDCT) < 0)
    {
        alert("ERROR", tjGetErrorStr2(decompressor));
        free(palette->buffer);
        free(palette);
        return NULL;
    }

    return palette;
}
