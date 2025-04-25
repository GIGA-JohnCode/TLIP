#include "util.h"
#include "tlip.h"

#include <ctype.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void show_error_dialog(GtkWindow *main_window, const char *error_message);

bool int_in_range(int value, int min_value, int max_value)
{
    if (value < min_value || value > max_value)
        return false;
    return true;
}

int get_int(const char *prompt)
{
    char *line = NULL;
    size_t line_len = 0;
    long value;

    while (true)
    {
        printf("%s", prompt);
        if (getline(&line, &line_len, stdin) == -1)
        {
            fprintf(stderr, "Error: Failed to read input.\n");
            continue;
        }

        char *endptr;
        errno = 0;
        value = strtol(line, &endptr, 10);

        bool trailing_spaces_only = true;
        for (char *ptr = endptr; *ptr != '\0'; ptr++)
            if (!isspace(*ptr))
            {
                trailing_spaces_only = false;
                break;
            }

        if (endptr == line)
            fprintf(stderr, "Error: No valid number found.\n");
        else if (!trailing_spaces_only)
            fprintf(stderr, "Error: Invalid trailing characters found.\n");
        else if (errno == ERANGE || value > INT_MAX || value < INT_MIN)
            fprintf(stderr, "Error: Value out of int range.\n");
        else
        {
            free(line);
            return (int)value;
        }
    }

    // Should never actually reach here
    free(line);
    return min_value;
}

void show_error(const char *error_message)
{
    if (cli_mode)
        fprintf(stderr, "Error: %s\n", error_message);
    else
        show_error_dialog(main_window, error_message);
}

static void show_error_dialog(GtkWindow *parent, const char *error_message)
{
    GtkWidget *dialog = gtk_message_dialog_new(parent,
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_OK,
                                               "Error");
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", error_message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
