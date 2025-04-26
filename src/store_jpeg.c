#include "jpeg.h"
#include "tlip.h"
#include "util.h"

#include <limits.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <turbojpeg.h>

static bool encode_jpeg(rgb* palette, int quality, unsigned char **jpeg_buffer, unsigned long *jpeg_size);
static bool write_jpeg(byte* jpeg_buffer, unsigned long jpeg_size, char* output_path);

bool store_jpeg(rgb* palette, size_t target_size, char* output_path)
{
    byte *jpeg_buffer = NULL;
    unsigned long jpeg_size = tjBufSize(palette->width, palette->height, TJSAMP_420);

    if (target_size < 0 || target_size > SIZE_MAX)
    {
        show_error("Invalid size");
        return false;
    }

    if (target_size == 0)
        target_size = SIZE_MAX;

    double reduction_needed = 1 - ((double)target_size / jpeg_size);

    int quality;
    if (reduction_needed <= 0)
        quality = 95;
    else if (reduction_needed <= 0.35)
        quality = 85;
    else if (reduction_needed <= 0.55)
        quality = 75;
    else
        quality = 65;

    bool failed = true;
    while (quality >= 30)
    {
        tjFree(jpeg_buffer); // Useless in first iteration
        jpeg_buffer = NULL;

        if (!encode_jpeg(palette, quality, &jpeg_buffer, &jpeg_size))
            return false;
        if (jpeg_size <= target_size)
        {
            failed = false;
            break;
        }

        quality -= 5;
    }
    if (failed)
        show_error("Could not meet target size. Attempting to save image at quality: 30."); // To Do: add a way to convey more info
    bool result = write_jpeg(jpeg_buffer, jpeg_size, output_path);
    tjFree(jpeg_buffer);
    return result;
}

static bool encode_jpeg(rgb* palette, int quality, unsigned char **jpeg_buffer, unsigned long *jpeg_size)
{
    tjhandle compressor = tjInitCompress();
    if (!compressor)
    {
        show_error(tjGetErrorStr2(NULL));
        return false;
    }

    int result = tjCompress2(compressor, palette->buffer, palette->width, 0, palette->height,
                            TJPF_RGB, jpeg_buffer, jpeg_size, TJSAMP_420, quality, TJFLAG_ACCURATEDCT);

    if (result != 0)
    {
        show_error(tjGetErrorStr2(compressor));
        tjFree(*jpeg_buffer);
        tjDestroy(compressor);
        return false;
    }
    tjDestroy(compressor);
    return true;
}

static bool write_jpeg(byte* jpeg_buffer, unsigned long jpeg_size, char* output_path)
{
    char* dot = strrchr(output_path, '.');
    if (!dot)
    {
        show_error("Given path: %s doesn't have an extension.");
        return false;
    }

    // To Do: make another param called default_path, check output_path validity
    //        choose default_path if invalid
    char unique_path[PATH_MAX];
    strcpy(unique_path, output_path);

    FILE *tester = NULL;
    while ((tester = fopen(unique_path, "r")) != NULL)
    {
        fclose(tester);

        char suffix[3] = {'_', 'a' + rand() % 26, '\0'};

        char base_path[PATH_MAX];
        memcpy(base_path, output_path, dot - output_path);
        base_path[dot - output_path] = '\0';

        char extension[PATH_MAX];
        strcpy(extension, dot);

        strcpy(unique_path, base_path);
        strcat(unique_path, suffix);
        strcat(unique_path, extension);
    }

    FILE *fp = fopen(unique_path, "wb");
    if (!fp)
    {
        show_error("Could not open file for writing."); // To Do: can be more elaborate
        return false;
    }

    if (fwrite(jpeg_buffer, 1, jpeg_size, fp) != jpeg_size)
    {
        show_error("Could not write JPEG data");
        fclose(fp);
        return false;
    }

    fclose(fp);
    alert("SUCCESS", "File saved to: %s\n", unique_path);
    return true;
}
