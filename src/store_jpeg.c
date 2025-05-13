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
static bool write_jpeg(byte* jpeg_buffer, unsigned long jpeg_size, char* output_path, char* input_path);

bool store_jpeg(rgb* palette, size_t target_size, char* output_path, char* input_path)
{
    byte *jpeg_buffer = NULL;
    unsigned long jpeg_size = tjBufSize(palette->width, palette->height, TJSAMP_420);

    if (target_size < 0 || target_size > SIZE_MAX)
    {
        alert("ERROR", "Invalid size");
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
        alert("WARNING", "Could not meet target size. Attempting to save image at quality: 30 (approx. %zu KB)",
              jpeg_size / 1024);
    bool result = write_jpeg(jpeg_buffer, jpeg_size, output_path, input_path);
    tjFree(jpeg_buffer);
    return result;
}

static bool encode_jpeg(rgb* palette, int quality, unsigned char **jpeg_buffer, unsigned long *jpeg_size)
{
    tjhandle compressor = tjInitCompress();
    if (!compressor)
    {
        alert("ERROR", tjGetErrorStr2(NULL));
        return false;
    }

    int result = tjCompress2(compressor, palette->buffer, palette->width, 0, palette->height,
                            TJPF_RGB, jpeg_buffer, jpeg_size, TJSAMP_420, quality, TJFLAG_ACCURATEDCT);

    if (result != 0)
    {
        alert("ERROR", tjGetErrorStr2(compressor));
        tjFree(*jpeg_buffer);
        tjDestroy(compressor);
        return false;
    }
    tjDestroy(compressor);
    return true;
}

static bool write_jpeg(byte* jpeg_buffer, unsigned long jpeg_size, char* output_path, char* input_path)
{
    if (!cli_mode || individual_input)
    {
        int path_status = evaluate_path(output_path);
        char output_dir[PATH_MAX];
        bool result = true;

        if (path_status == -1)
        {
            strcpy(output_dir, output_path);
            char *last_sep = strrchr(output_dir, PATH_SEP);
            *last_sep = '\0';

            if (!mkdir_p(output_dir))
                result = false;
            path_status = evaluate_path(output_path);
        }

        if (path_status == 2)
        {
            strcpy(output_dir, output_path);
            size_t len = strlen(output_dir);

            if (output_dir[len - 1] == PATH_SEP)
                output_dir[len - 1] = '\0';
            char *image_name = strrchr(input_path, PATH_SEP);
            if (!image_name)
                image_name = input_path;
            else
                image_name++;
            snprintf(output_path, PATH_MAX, "%s%c%s", output_dir, PATH_SEP, image_name);
            path_status = evaluate_path(output_path);
        }

        if (path_status == 1)
            result = confirm("File: %s already exists.\nOverwrite?", output_path);

        if (!result || path_status == -2)
            get_duplicate_path(output_path, input_path);
    }

    FILE *fptr = fopen(output_path, "wb");
    if (!fptr)
    {
        alert("ERROR", "File: %s could not be opened for writing.", output_path);
        return false;
    }

    if (fwrite(jpeg_buffer, 1, jpeg_size, fptr) != jpeg_size)
    {
        alert("ERROR", "Could not write JPEG data");
        fclose(fptr);
        return false;
    }

    fclose(fptr);
    if (individual_input)
    {
        if (confirm("File successfully saved to: %s\nDo you want to open it?", output_path))
            view_image(output_path);
    }
    else
        alert("SUCCESS", "File saved to: %s", output_path);
    return true;
}
