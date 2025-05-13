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

#ifdef _WIN32
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <sys/stat.h>
    #include <sys/types.h>
#endif

#define ALERT_MAX 8192
#define RETRIES_MAX 100

typedef struct
{
    gboolean *response;
    GMainLoop *loop;
} confirm_response_data;

static char view_image_filename[PATH_MAX];

static void alert_done_callback(GObject *source, GAsyncResult *result, gpointer user_data);
static void show_alert_dialog(GtkWindow *parent, const char *type, const char *message);
static void confirm_done_callback(GObject *source, GAsyncResult *result, gpointer user_data);
static bool show_confirm_dialog(GtkWindow *parent, const char *message);
static void on_view_image_activate(GtkApplication *app, gpointer user_data);

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

void alert(const char *type, const char *format, ...)
{
    char message[ALERT_MAX];

    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    if (cli_mode)
        fprintf(stderr, "%s: %s\n", type, message);
    else
        show_alert_dialog(main_window, type, message);
}

int evaluate_path(char *path)
{
    if (path[0] == '\0' || path[0] == '\n')
        return -2; // user chose not to enter path

    struct stat s;
    if (stat(path, &s) == -1)
    {
        char dir_path[PATH_MAX];
        strcpy(dir_path, path);
        char *last_slash = strrchr(dir_path, PATH_SEP);
        if (last_slash)
            *last_slash = '\0';
        else
            strcpy(dir_path, "."); // handles case when user wants to create new file in current directory

        DIR *dir = opendir(dir_path);
        if (!dir)
            return -1; // path was garbage
        else
        {
            closedir(dir);
            return 0; // file doesn't yet exists but can be created
        }
    }

    if (S_ISREG(s.st_mode))
        return 1; // file exists
    if (S_ISDIR(s.st_mode))
        return 2; // directory exists

    return INT_MIN; // should never really reach here, but compiler just won't shut up
}

bool get_duplicate_path(char* output_path, char* input_path)
{
    char base[PATH_MAX];
    char ext[20] = "";
    char temp[PATH_MAX];
    strcpy(temp, input_path);

    char *dot = strrchr(temp, '.');
    strcpy(ext, dot);
    *dot = '\0';
    strcpy(base, temp);

    int counter = 0;
    do
    {
        snprintf(output_path, PATH_MAX, "%s(%d)%s", base, ++counter, ext);

        FILE *file = fopen(output_path, "r");
        if (!file)
            return true;
        fclose(file);
    }
    while (counter < RETRIES_MAX);
    return false;
}

void parse_args(int argc, char *argv[], params *inputs)
{
    if (argc <= 2)
        return;
    if (argv[2][0] == '"')
    {
        if (!unquote_cpy(inputs->src, argv[2]))
        {
            alert("ERROR", "Missing closing \" in argument");
            return;
        }
    }
    else
        strcpy(inputs->src, argv[2]);

    get_img_path_list(inputs);

    if (argc > 3)
    {
        int temp;
        if (strcmp(argv[3], "") == 0)
            inputs->width = INT_MIN;
        else
        {
            temp = atoi(argv[3]);
            inputs->width = (temp > 0) ? temp : -1;
        }
        if (argc > 4)
        {
            if (strcmp(argv[4], "") == 0)
                inputs->height = INT_MIN;
            else
            {
                temp = atoi(argv[4]);
                inputs->height = (temp > 0) ? temp : -1;
            }
            if (argc > 5)
            {
                if (strcmp(argv[5], "") == 0)
                    inputs->target_size = INT_MIN;
                else
                {
                    temp = atoi(argv[5]);
                    inputs->target_size = (temp >= 0) ? temp : -1;
                }
                if (argc > 6)
                {
                    if (argv[6][0] == '"')
                    {
                        if (!unquote_cpy(inputs->dest, argv[6]))
                        {
                            alert("ERROR", "Missing closing \" in argument");
                            return;
                        }
                    }
                    else
                        strcpy(inputs->dest, argv[6]);
                }
            }
        }
    }
}

bool unquote_cpy(char *path, char arg[])
{
    char *quotation = strrchr(arg, '"');
    if (quotation && quotation != arg)
    {
        *quotation = '\0';
        strcpy(path, arg + 1);
        return true;
    }
    else
        return false;
}

void get_img_path_list(params *inputs)
{
    char *path = inputs->src;
    int path_status = evaluate_path(path);
    if (path_status == 1)
    {
        if (!is_jpeg(path))
            return;
        inputs->img_paths = malloc(2 * sizeof(char*));
        if (!(inputs->img_paths))
        {
            alert("ERROR", "Memory allocation failed.");
            return;
        }
        inputs->img_paths[0] = strdup(path);
        if (!inputs->img_paths[0])
        {
            free_path_list(inputs->img_paths);
            alert("ERROR", "Memory allocation failed.");
            return;
        }
        inputs->img_paths[1] = NULL;
        inputs->path_count = 1;
    }
    else if (path_status == 2)
    {
        DIR *dir = opendir(path);
        if (!dir)
        {
            alert("ERROR", "Could not open directory: %s", path);
            return;
        }
        struct dirent *entry;

        int jpeg_count = 0;
        char entry_path[PATH_MAX];
        while ((entry = readdir(dir)) != NULL)
        {
            snprintf(entry_path, PATH_MAX, "%s%c%s", path, PATH_SEP, entry->d_name);
            if (is_jpeg(entry_path))
                jpeg_count++;
        }
        if (jpeg_count == 0)
        {
            closedir(dir);
            alert("ERROR", "No JPEG files found in directory: %s", path);
            return;
        }

        inputs->img_paths = malloc((jpeg_count + 1) * sizeof(char*));
        if (!(inputs->img_paths))
        {
            closedir(dir);
            alert("ERROR", "Memory allocation failed.");
            return;
        }
        rewinddir(dir);
        for (int i = 0; (entry = readdir(dir)) != NULL && i < jpeg_count;)
        {
            snprintf(entry_path, PATH_MAX, "%s%c%s", path, PATH_SEP, entry->d_name);
            if (is_jpeg(entry_path))
            {
                inputs->img_paths[i] = strdup(entry_path);
                if (!inputs->img_paths[i])
                {
                    free_path_list(inputs->img_paths);
                    closedir(dir);
                    alert("ERROR", "Memory allocation failed");
                    return;
                }
                i++;
            }
        }
        inputs->img_paths[jpeg_count] = NULL;
        inputs->path_count = jpeg_count;
    }
    else if (path_status == -2)
        alert("ERROR", "No path provided");
    else if (path_status == -1)
        alert("ERROR", "Invalid path: %s", path);
    else
        alert("ERROR", "Path is not a file or directory: %s", path);
}

bool is_jpeg(char* path)
{
    FILE* file = fopen(path, "rb");
    if (file == NULL)
        return false;

    byte marker[2];
    if (fread(marker, 1, 2, file) != 2)
    {
        fclose(file);
        return false;
    }
    fclose(file);
    if (marker[0] == 0xFF && marker[1] == 0xD8)
        return true;
    return false;
}

void free_path_list(char **path_list)
{
    if (!path_list)
        return;

    for (int i = 0; path_list[i]; i++)
        free(path_list[i]);
    free(path_list);
}

bool get_duplicate_dir(char* output_dir)
{
    char temp[PATH_MAX];
    strcpy(temp, output_dir);

    int counter = 0;
    do
    {
        snprintf(output_dir, PATH_MAX, "%s(%d)", temp, ++counter);
        DIR *dir = opendir(output_dir);
        if (!dir)
            return true;
        closedir(dir);
    }
    while (counter < RETRIES_MAX);
    return false;
}

bool mkdir_p(const char *path)
{
    struct stat st;
    char temp[PATH_MAX];
    size_t len = strlen(path);
    if (len >= PATH_MAX)
        return false;

    strcpy(temp, path);

    for (char *p = temp + 1; *p; p++)
    {
        if (*p == PATH_SEP)
        {
            *p = '\0';
            if (access(temp, F_OK) != 0)
            {
                if (mkdir(temp, 0755) == -1)
                    return false;
            }
            else if (!(stat(temp, &st) == 0 && S_ISDIR(st.st_mode)))
                return false;
            *p = PATH_SEP;
        }
    }

    if (access(temp, F_OK) != 0 && mkdir(temp, 0755) == -1)
        return false;
    return true;
}

bool confirm(const char *format, ...)
{
    char message[ALERT_MAX];

    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    if (cli_mode)
    {
        printf("%s [y/N]: ", message);

        char response[10];
        if (fgets(response, sizeof(response), stdin))
        {
            if (response[0] == 'y' || response[0] == 'Y')
                return true;
            else
                return false;
        }
        else
            return false;
    }
    else
        return show_confirm_dialog(main_window, message);
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

static bool show_confirm_dialog(GtkWindow *parent, const char *message)
{
    gboolean response = FALSE;

    GtkAlertDialog *dialog = gtk_alert_dialog_new("Confirm");
    gtk_alert_dialog_set_detail(dialog, message);

    gtk_alert_dialog_set_buttons(dialog, (const char*[]){"No", "Yes", NULL});
    gtk_alert_dialog_set_cancel_button(dialog, 0);
    gtk_alert_dialog_set_default_button(dialog, 1);

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    confirm_response_data data = { &response, loop };

    gtk_alert_dialog_choose(dialog, parent, NULL,
                           (GAsyncReadyCallback)confirm_done_callback, &data);

    g_main_loop_run(loop);
    g_main_loop_unref(loop);
    g_object_unref(dialog);

    return response;
}

static void confirm_done_callback(GObject *source, GAsyncResult *result, gpointer user_data)
{
    confirm_response_data *data = (confirm_response_data *)user_data;
    int button_index = gtk_alert_dialog_choose_finish(GTK_ALERT_DIALOG(source), result, NULL);

    *(data->response) = (button_index == 1);

    g_main_loop_quit(data->loop);
}

bool view_image(const char *filename)
{
    strncpy(view_image_filename, filename, PATH_MAX - 1);
    view_image_filename[PATH_MAX - 1] = '\0';

    GtkApplication *app = gtk_application_new("com.example.tlip.viewer", G_APPLICATION_NON_UNIQUE);

    g_signal_connect(app, "activate", G_CALLBACK(on_view_image_activate), NULL);

    int status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);

    return (status == 0);
}

static void on_view_image_activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), view_image_filename);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(view_image_filename, NULL);
    if (!pixbuf)
    {
        alert("ERROR", "Failed to load image: %s", view_image_filename);
        return;
    }

    GtkWidget *picture = gtk_picture_new();
    GdkPaintable *paintable = GDK_PAINTABLE(gdk_texture_new_for_pixbuf(pixbuf));
    gtk_picture_set_paintable(GTK_PICTURE(picture), paintable);
    gtk_picture_set_can_shrink(GTK_PICTURE(picture), TRUE);
    gtk_picture_set_content_fit(GTK_PICTURE(picture), GTK_CONTENT_FIT_CONTAIN);

    g_object_unref(paintable);
    g_object_unref(pixbuf);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), picture);
    gtk_window_set_child(GTK_WINDOW(window), scrolled);
    gtk_window_present(GTK_WINDOW(window));
}


