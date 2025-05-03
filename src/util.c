#include "util.h"
#include "tlip.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALERT_MAX 8192

static void alert_done_callback(GObject *source, GAsyncResult *result, gpointer user_data);
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
    char message[ALERT_MAX];

    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    if (cli_mode)
        fprintf(stderr, "%s: %s", type, message);
    else
        show_alert_dialog(main_window, type, message);
}

static void show_alert_dialog(GtkWindow *parent, const char *type, const char *message)
{
    GtkAlertDialog *dialog = gtk_alert_dialog_new("%s", type);
    gtk_alert_dialog_set_detail(dialog, message);
    gtk_alert_dialog_set_buttons(dialog, (const char*[]){"OK", NULL});
    gtk_alert_dialog_set_modal(dialog, TRUE);

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    gtk_alert_dialog_choose(dialog, parent, NULL, (GAsyncReadyCallback)alert_done_callback, loop);
    g_main_loop_run(loop);

    g_main_loop_unref(loop);
    g_object_unref(dialog);
}

static void alert_done_callback(GObject *source, GAsyncResult *result, gpointer user_data)
{
    GMainLoop *loop = (GMainLoop *)user_data;
    g_main_loop_quit(loop);
}

int evaluate_path(char *path)
{
    if (path[0] == '\0' || path[0] == '\n')
        return -2;
    FILE *file = fopen(path, "r");
    if (file)
    {
        fclose(file);
        return 1;
    }

    errno = 0;
    char dir_path[PATH_MAX];
    strcpy(dir_path, path);
    char *last_slash = strrchr(dir_path, '/');
    if (last_slash)
        *last_slash = '\0';
    else
        strcpy(dir_path, ".");

    DIR *dir = opendir(dir_path);
    if (!dir)
        return -1;
    else
    {
        closedir(dir);
        return 0;
    }
}

