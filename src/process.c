#include "process.h"
#include "util.h"

#include <stdbool.h>
#include <stdlib.h>

bool resize(rgb* palette, int new_width, int new_height)
{
    int old_width = palette->width;
    int old_height = palette->height;
    int components = palette->components;

    if (new_width < 1 || new_height < 1)
    {
        alert("ERROR", "Invalid dimensions.");
        return false;
    }

    if (old_width == new_width && old_height == new_height)
        return true;

    byte *temp_buffer = (byte*)malloc(new_width * new_height * components);
    if (!temp_buffer)
    {
        alert("ERROR", "Memory allocation failed.");
        return false;
    }

    float x_ratio = (float)(old_width - 1) / (new_width - 1);
    float y_ratio = (float)(old_height - 1) / (new_height - 1);

    for (int y = 0; y < new_height; y++)
    {
        for (int x = 0; x < new_width; x++)
        {
            float src_x = x * x_ratio;
            float src_y = y * y_ratio;

            int x1 = (int)src_x;
            int x2 = (x1 == old_width - 1)? x1 : (x1 + 1);
            int y1 = (int)src_y;
            int y2 = (y1 == old_height - 1)? y1 : (y1 + 1);

            float x_weight = src_x - x1;
            float y_weight = src_y - y1;

            for (int c = 0; c < components; c++)
            {
                int index_11 = (y1 * old_width + x1) * components + c;
                int index_12 = (y1 * old_width + x2) * components + c;
                int index_21 = (y2 * old_width + x1) * components + c;
                int index_22 = (y2 * old_width + x2) * components + c;

                byte p11 = palette->buffer[index_11];
                byte p12 = palette->buffer[index_12];
                byte p21 = palette->buffer[index_21];
                byte p22 = palette->buffer[index_22];

                float top = p11 * (1 - x_weight) + p12 * x_weight;
                float bottom = p21 * (1 - x_weight) + p22 * x_weight;
                byte value = (byte)(top * (1 - y_weight) + bottom * y_weight);

                temp_buffer[(y * new_width + x) * components + c] = value;
            }
        }
    }

        free(palette->buffer);
        palette->buffer = temp_buffer;
        palette->width = new_width;
        palette->height = new_height;
        return true;
}
