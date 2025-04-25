#include "tlip.h"

#include <linux/limits.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int cli_main(void)
{
    // Prepare to get image path
    char *jpeg_path = NULL;
    size_t line_buf_size = 0;

    // Get image path from user
    while (true)
    {
        printf("Enter image path: ");
        if (getline(&jpeg_path, &line_buf_size, stdin) == -1)
            show_error("Failed to read image path, please try again.");
        else
            break;
    }

    // Post-process the path
    size_t len = strlen(jpeg_path);
    if (len > 0 && jpeg_path[len - 1] == '\n')
            jpeg_path[len - 1] = '\0';

    printf("Loading image: %s\n", jpeg_path);
    rgb* palette = load_jpeg(jpeg_path);
    if (!palette)
    {
        free(jpeg_path);
        return -1;
    }

    printf("Current width: %i, ", palette->width);
    int new_width = get_int("Enter new width: ", 1, INT_MAX);
    printf("Current height: %i, ", palette->height);
    int new_height = get_int("Enter new height: ", 1, INT_MAX);

    if (!resize(palette, new_width, new_height))
    {
        free(jpeg_path);
        return -1;
    }

    srand(time(NULL));
    size_t target_size = 1024 * get_int("Enter max jpeg size in KB: ", 1, INT_MAX);

    printf("Enter output path (or press enter to use default: %s): ", jpeg_path);
    char output_path[PATH_MAX];

    if (!fgets(output_path, PATH_MAX, stdin))
    {
        show_error("Failed to read output path.");
        free(jpeg_path);
        return -1;
    }

    len = strlen(output_path);
    if (len > 0 && output_path[len - 1] == '\n')
        output_path[len - 1] = '\0';

    if (output_path[0] == '\0')
        strcpy(output_path, jpeg_path);

    bool result = store_jpeg(palette, target_size, output_path);
    free(palette->buffer);
    free(palette);
    free(jpeg_path);
    if (!result)
        return -1;
    else
        return 0;
}
