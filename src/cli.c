#include "tlip.h"

#include <dirent.h>
#include <linux/limits.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void fill_params(params *inputs);

int cli_main(int argc, char *argv[])
{
    params inputs;
    inputs.src[0] = '\0';
    inputs.dest[0] = '\0';
    inputs.img_paths = NULL;
    inputs.path_count = -1;
    inputs.width = -1;
    inputs.height = -1;
    inputs.target_size = -1;

    parse_args(argc, argv, &inputs);
    fill_params(&inputs);

    char output_dir[PATH_MAX];
    strcpy(output_dir, inputs.dest);
    if (!individual_input)
    {
        for (int i = 0; true; i++)
        {
            if (evaluate_path(output_dir) == 2)
            {
                printf("# Directory already exists\n");
                if (confirm("Any file/s in this directory with the same name & ext will be overwritten, are you sure?"))
                    break;
                else if (confirm("Do you want to create a duplicate directory instead?"))
                {
                    if (!get_duplicate_dir(output_dir))
                        continue;
                }
                else
                    continue;
            }
            if (evaluate_path(output_dir) == -1 || evaluate_path(output_dir) == 0)
            {
                if (mkdir_p(output_dir))
                    break;
                else
                    continue;
            }

            if (i == 0)
            {
                printf("# Enter directory path where you want to store output files.\n");
                printf("# Alternatively press Enter to create & select tlip_output subdirectory in the input directory.\n");
            }
            printf("Enter output directory: ");
            if (!fgets(output_dir, PATH_MAX, stdin))
            {
                free_path_list(inputs.img_paths);
                return -1;
            }
            size_t len = strlen(output_dir);

            if (len == 1 && output_dir[len - 1] == '\n')
            {
                strcpy(output_dir, inputs.src);
                len = strlen(output_dir);
                if (output_dir[len - 2] == PATH_SEP)
                    output_dir[len - 2] = '\0';
                char temp[PATH_MAX];
                strcpy(temp, output_dir);
                if (!snprintf(output_dir, PATH_MAX, "%s%ctlip_output", temp, PATH_SEP))
                    continue;
            }
            else if (len > 0 && output_dir[len - 1] == '\n')
                output_dir[len - 1] = '\0';
        }
    }

    char input_path[PATH_MAX];
    for (int i = 0; i < inputs.path_count; i++)
    {
        strcpy(input_path, inputs.img_paths[i]);
        printf("[%i/%i] Loading image: %s ", i + 1, inputs.path_count, input_path);
        rgb* palette = load_jpeg(input_path);
        if (!palette)
        {
            printf("Failed.\n");
            continue;
        }
        else
            printf("Done.\n");

        if (individual_input)
        {
            if (inputs.width == -1 || i > 0)
            {
                printf("# Current width: %i px (Press Enter to keep current width)\n", palette->width);
                inputs.width = get_int("Enter new width: ");
            }
            if (inputs.height == -1 || i > 0)
            {
                printf("# Current height: %i px (Press Enter to keep current height)\n", palette->height);
                inputs.height = get_int("Enter new height: ");
            }
            if (inputs.target_size == -1 || i > 0)
            {
                printf("# Current size: %.1f KB (Press Enter to keep current size or 0 to have no limit)\n",
                        palette->original_size / 1024.0);
                inputs.target_size = get_int("Enter max jpeg size in KB: ");
            }
        }

        if (inputs.width == INT_MIN && inputs.height == INT_MIN && inputs.target_size == INT_MIN)
        {
            alert("SKIPPED", "Nothing to do.");
            free(palette->buffer);
            free(palette);
            if (individual_input)
                continue;
            else
                break;
        }

        if (individual_input || i == 0)
        {
            if (inputs.width == INT_MIN)
                inputs.width = palette->width;
            if (inputs.height == INT_MIN)
                inputs.height = palette->height;
            if (inputs.target_size == INT_MIN)
                inputs.target_size = palette->original_size;
            else
                inputs.target_size *= 1024;
        }

        if (!resize(palette, inputs.width, inputs.height))
        {
            free(palette->buffer);
            free(palette);
            continue;
        }

        char *image_name = strrchr(input_path, PATH_SEP);
        if (!image_name)
            image_name = input_path;
        else
            image_name++;

        char output_path[PATH_MAX];
        if (individual_input)
        {
            if (inputs.dest[0] == '\0' || i > 0)
            {
                printf("Enter output path (or press enter to save as duplicate):\n");
                if (!fgets(output_path, PATH_MAX, stdin))
                {
                    alert("ERROR", "Failed to read output path.");
                    free(palette->buffer);
                    free(palette);
                    continue;
                }
                size_t len = strlen(output_path);
                if (len > 0 && output_path[len - 1] == '\n')
                    output_path[len - 1] = '\0';
            }
            else
                strcpy(output_path, inputs.dest);
        }
        else
        {
            size_t len = strlen(output_dir);
            if (output_dir[len - 2] == PATH_SEP)
                output_dir[len - 2] = '\0';
            snprintf(output_path, PATH_MAX, "%s%c%s", output_dir, PATH_SEP, image_name);
        }
        store_jpeg(palette, inputs.target_size, output_path, input_path);
        free(palette->buffer);
        free(palette);
    }
    free_path_list(inputs.img_paths);
    return 0;
}

static void fill_params(params *inputs)
{
    while (inputs->img_paths == NULL)
    {
        printf("Enter image path or directory: ");
        if (!fgets(inputs->src, PATH_MAX, stdin))
        {
            fprintf(stderr, "Couldn't read input path");
            continue;
        }
        size_t len = strlen(inputs->src);
        if (len > 0 && inputs->src[len - 1] == '\n')
            inputs->src[len - 1] = '\0';

        get_img_path_list(inputs);
    }
    printf("Successfully Loaded %i images.\n", inputs->path_count);
    if (inputs->path_count > 1 && !confirm("Manually enter processing parameters for each image?"))
        individual_input = false;

    if (!individual_input)
    {
        int temp;
        while (inputs->width == -1)
        {
            printf("# Press Enter to keep current width\n");
            temp = get_int("New width: ");
            if (int_in_range(temp, 1, INT_MAX) || temp == INT_MIN)
                inputs->width = temp;
        }
        while (inputs->height == -1)
        {
            printf("# Press Enter to keep current height\n");
            temp = get_int("New height: ");
            if (int_in_range(temp, 1, INT_MAX) || temp == INT_MIN)
                inputs->height = temp;
        }
        while (inputs->target_size == -1)
        {
            printf("# Press Enter to keep current size or 0 to have no limit\n");
            temp = get_int("target_size: ");
            if (int_in_range(temp, 0, INT_MAX) || temp == INT_MIN)
                inputs->target_size = temp;
        }
    }
}
