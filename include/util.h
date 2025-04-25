#pragma once

int get_int(const char *prompt);
bool int_in_range(int value, int min_value, int max_value);
void show_error(const char *error_message);
