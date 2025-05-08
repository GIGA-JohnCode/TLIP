#pragma once

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
