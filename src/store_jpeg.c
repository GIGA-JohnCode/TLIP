#include "jpeg.h"

#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <turbojpeg.h>

static bool encode_jpeg(rgb* palette, int quality, unsigned char **jpeg_buffer, unsigned long *jpeg_size);
static bool write_jpeg(byte* jpeg_buffer, unsigned long jpeg_size, char* default_path);

bool store_jpeg(rgb* palette, size_t target_size, char* default_path)
{
    byte *jpeg_buffer = NULL;
    unsigned long jpeg_size = tjBufSize(palette->width, palette->height, TJSAMP_420);

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
        fprintf(stderr, "Warning: Could not meet target size %zu bytes.\n"
                "Attempting to save image of size: %lu bytes at quality: 30.\n",
                target_size, jpeg_size);

    bool result = write_jpeg(jpeg_buffer, jpeg_size, default_path);
    tjFree(jpeg_buffer);
    return result;
}

bool encode_jpeg(rgb* palette, int quality, unsigned char **jpeg_buffer, unsigned long *jpeg_size)
{
    tjhandle compressor = tjInitCompress();
    if (!compressor)
    {
        fprintf(stderr, "Error: TurboJPEG initialization failed: %s\n", tjGetErrorStr2(NULL));
        return false;
    }

    int result = tjCompress2(compressor, palette->buffer, palette->width, 0, palette->height,
                            TJPF_RGB, jpeg_buffer, jpeg_size, TJSAMP_420, quality, TJFLAG_ACCURATEDCT);

    if (result != 0)
    {
        fprintf(stderr, "Error: JPEG compression failed: %s\n", tjGetErrorStr2(compressor));
        tjFree(*jpeg_buffer);
        tjDestroy(compressor);
        return false;
    }
    tjDestroy(compressor);
    return true;
}

bool write_jpeg(byte* jpeg_buffer, unsigned long jpeg_size, char* default_path)
{
    printf("Enter output path (or press enter to use default: %s): ", default_path);
    char output_path[PATH_MAX];

    if (!fgets(output_path, PATH_MAX, stdin))
    {
        fprintf(stderr, "Error: Failed to read output path.\n");
        return false;
    }

    size_t len = strlen(output_path);
    if (len > 0 && output_path[len - 1] == '\n')
        output_path[len - 1] = '\0';

    if (output_path[0] == '\0')
        strcpy(output_path, default_path);

    char* dot = strrchr(output_path, '.');
    if (!dot)
    {
        fprintf(stderr, "Error: Given path: %s doesn't have an extension.\n", output_path);
        return false;
    }

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
        fprintf(stderr, "Error: Could not open file for writing: %s\n", unique_path);
        return false;
    }

    if (fwrite(jpeg_buffer, 1, jpeg_size, fp) != jpeg_size)
    {
        fprintf(stderr, "Error: Could not write JPEG data\n");
        fclose(fp);
        return false;
    }

    fclose(fp);
    printf("Successfully saved JPEG to: %s\n", unique_path);
    return true;
}
