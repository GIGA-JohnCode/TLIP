#include "util.h"
#include "tlip.h"

#include <ctype.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALERT_MAX 8192

static void show_alert_dialog(GtkWindow *parent, const char *type, const char *message);

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

        // Enable user the choice to not input any value
        if (line[0] == '\n')
        {
            free(line);
            return INT_MIN;
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
    return value;
}

void show_error(const char *message)
{
    alert("ERROR", "%s", message);
}

void alert(const char *type, const char *format, ...)
{
    char message[8192];

    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    if (cli_mode)
        fprintf(stderr, "%s: %s\n", type, message);
    else
        show_alert_dialog(main_window, type, message);
}

static void show_alert_dialog(GtkWindow *parent, const char *type, const char *message)
{
    GtkAlertDialog *dialog = gtk_alert_dialog_new("%s", type);
    gtk_alert_dialog_set_detail(dialog, message);
    gtk_alert_dialog_set_buttons(dialog, (const char*[]){"OK", NULL});
    gtk_alert_dialog_set_modal(dialog, TRUE);
    gtk_alert_dialog_show(dialog, parent);
    g_object_unref(dialog);
}

