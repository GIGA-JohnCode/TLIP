#include "tlip.h"

#include <linux/limits.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cli_main(void)
{
    // Prepare to get image path
    char *input_path = NULL;
    size_t line_buf_size = 0;

    // Get image path from user
    while (true)
    {
        printf("Enter image path: ");
        if (getline(&input_path, &line_buf_size, stdin) == -1)
            alert("ERROR", "Failed to read image path, please try again.");
        else
            break;
    }

    // Post-process the path
    size_t len = strlen(input_path);
    if (len > 0 && input_path[len - 1] == '\n')
            input_path[len - 1] = '\0';

    printf("Loading image: %s\n", input_path);
    rgb* palette = load_jpeg(input_path);
    if (!palette)
    {
        free(input_path);
        return -1;
    }

    printf("Current width: %i px (Press Enter to keep current width)\n", palette->width);
    int new_width = get_int("Enter new width: ");
    if (new_width == INT_MIN)
        new_width = palette->width;

    printf("Current height: %i px (Press Enter to keep current height)\n", palette->height);
    int new_height = get_int("Enter new height: ");
    if (new_height == INT_MIN)
        new_height = palette->height;

    printf("Current size: %.1f KB (Press Enter to keep current size or 0 to have no limit)\n", palette->original_size / 1024.0);
    size_t target_size = get_int("Enter max jpeg size in KB: ");
    if (target_size == INT_MIN)
        target_size = palette->original_size;
    else
        target_size *= 1024;

    if (new_width == palette->width && new_height == palette->height && target_size == palette->original_size)
    {
        alert("ERROR", "Nothing to do.");
        free(palette->buffer);
        free(palette);
        free(input_path);
        return -1;
    }

    if (!resize(palette, new_width, new_height))
    {
        free(palette->buffer);
        free(palette);
        free(input_path);
        return -1;
    }

    printf("Enter output path (or press enter to save as duplicate):\n");
    char output_path[PATH_MAX];

    if (!fgets(output_path, PATH_MAX, stdin))
    {
        alert("ERROR", "Failed to read output path.");
        free(palette->buffer);
        free(palette);
        free(input_path);
        return -1;
    }

    len = strlen(output_path);
    if (len > 0 && output_path[len - 1] == '\n')
        output_path[len - 1] = '\0';

    if (output_path[0] == '\0')
        strcpy(output_path, input_path);

    bool result = store_jpeg(palette, target_size, output_path, input_path);
    free(palette->buffer);
    free(palette);
    free(input_path);
    if (!result)
        return -1;
    else
        return 0;
}
