#include "jpeg.h"
#include "util.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <turbojpeg.h>

rgb* load_jpeg(char *jpeg_path)
{
    jpeg* image = read_jpeg(jpeg_path);
    if (!image)
        return NULL;

    tjhandle decompressor = tjInitDecompress();
    if (!decompressor)
    {
        fprintf(stderr, "Error: TurboJPEG initialization failed: %s\n", tjGetErrorStr2(NULL));
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

jpeg* read_jpeg(char *jpeg_path)
{
    jpeg *image = (jpeg*)malloc(sizeof(jpeg));
    if (!image)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }

    FILE *fptr = fopen(jpeg_path, "rb");
    if (!fptr)
    {
        fprintf(stderr, "Error: Failed to open image at given path.\n");
        free(image);
        return NULL;
    }

    fseek(fptr, 0, SEEK_END);
    image->size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    image->buffer = (byte*)malloc(sizeof(byte) * image->size);
    if (!image->buffer)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        free(image);
        fclose(fptr);
        return NULL;
    }

    if (fread(image->buffer, 1, image->size, fptr) != image->size)
    {
        fprintf(stderr, "Error: Failed to read image at given path.\n");
        free(image);
        fclose(fptr);
        return NULL;
    }

    fclose(fptr);
    return image;
}

bool parse_jpeg(jpeg* image, tjhandle decompressor)
{
    if (tjDecompressHeader3(decompressor, image->buffer, image->size,
       &(image->width), &(image->height), &(image->subsamp), &(image->colorspace)) < 0)
    {
        fprintf(stderr, "Error: Failed to read JPEG header: %s\n", tjGetErrorStr2(decompressor));
        return false;
    }
    return true;
}

rgb* extract_rgb(jpeg* image, tjhandle decompressor)
{
    rgb* palette = (rgb*)malloc(sizeof(rgb));
    if (!palette)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }

    palette->width = image->width;
    palette->height = image->height;
    palette->components = 3;

    palette->buffer = (byte*)malloc(palette->width * palette->height * palette->components);
    if (!palette->buffer)
    {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        free(palette);
        return NULL;
    }

    if (tjDecompress2(decompressor, image->buffer, image->size,
        palette->buffer, palette->width, 0, palette->height, TJPF_RGB, TJFLAG_ACCURATEDCT) < 0)
    {
        fprintf(stderr, "Error: JPEG decompression failed: %s\n", tjGetErrorStr2(decompressor));
        free(palette->buffer);
        free(palette);
        return NULL;
    }

    return palette;
}

