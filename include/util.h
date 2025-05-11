#pragma once

#include "types.h"

#include <stdbool.h>

#ifdef _WIN32
    #define PATH_SEP '\\'
#else
    #define PATH_SEP '/'
#endif

int get_int(const char *prompt);
bool int_in_range(int value, int min_value, int max_value);
void alert(const char *type, const char *format, ...);
int evaluate_path(char *path);
bool get_duplicate_path(char* output_path, char* input_path);
bool confirm(const char *format, ...);
bool get_duplicate_dir(char* output_dir);
bool mkdir_p(const char *path);
void parse_args(int argc, char *argv[], params *inputs);
bool unquote_cpy(char *path, char arg[]);
void get_img_path_list(params *inputs);
bool is_jpeg(char* path);
void free_path_list(char **path_list);
bool view_image(const char *filename);
