#define _XOPEN_SOURCE 700
#define _DEFAULT_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>     // for PATH_MAX
#include <errno.h>
#include <ctype.h>

// ANSI color codes
#define COLOR_RESET   "\033[0m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_RED     "\033[0;31m"
#define COLOR_PINK    "\033[0;35m"
#define COLOR_REVERSE "\033[7m"

// Function prototypes
void do_ls(const char *dirname, int long_format, int column_mode, int recursive_flag);
int compare_names(const void *a, const void *b);
void print_colored(const char *name, const char *path);
void display_entry(const char *dirname, const char *name, int long_format);

// Comparison function for qsort
int compare_names(const void *a, const void *b) {
    const char *nameA = *(const char **)a;
    const char *nameB = *(const char **)b;
    return strcmp(nameA, nameB);
}

// Print with color depending on file type
void print_colored(const char *name, const char *path) {
    struct stat st;
    char fullpath[PATH_MAX];

    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, name);

    if (lstat(fullpath, &st) == -1) {
        perror("lstat");
        printf("%s", name);
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        printf(COLOR_BLUE "%s" COLOR_RESET, name);
    } else if (S_ISLNK(st.st_mode)) {
        printf(COLOR_PINK "%s" COLOR_RESET, name);
    } else if (S_ISCHR(st.st_mode) || S_ISSOCK(st.st_mode)) {
        printf(COLOR_REVERSE "%s" COLOR_RESET, name);
    } else if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
        printf(COLOR_GREEN "%s" COLOR_RESET, name);
    } else if (strstr(name, ".tar") || strstr(name, ".gz") || strstr(name, ".zip")) {
        printf(COLOR_RED "%s" COLOR_RESET, name);
    } else {
        printf("%s", name);
    }
}

// Core recursive ls function
void do_ls(const char *dirname, int long_format, int column_mode, int recursive_flag) {
    DIR *dir = opendir(dirname);
    if (!dir) {
        perror(dirname);
        return;
    }

    struct dirent *entry;
    char **filenames = NULL;
    size_t count = 0;

    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        filenames = realloc(filenames, (count + 1) * sizeof(char *));
        filenames[count] = strdup(entry->d_name);
        count++;
    }
    closedir(dir);

    qsort(filenames, count, sizeof(char *), compare_names);

    printf("\n%s:\n", dirname);
    for (size_t i = 0; i < count; i++) {
        print_colored(filenames[i], dirname);
        printf("\n");
    }

    // Recursive part
    if (recursive_flag) {
        for (size_t i = 0; i < count; i++) {
            char fullpath[PATH_MAX];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, filenames[i]);

            struct stat st;
            if (lstat(fullpath, &st) == 0 && S_ISDIR(st.st_mode)) {
                do_ls(fullpath, long_format, column_mode, recursive_flag);
            }
        }
    }

    for (size_t i = 0; i < count; i++)
        free(filenames[i]);
    free(filenames);
}

int main(int argc, char *argv[]) {
    int opt;
    int long_format = 0, column_mode = 0, recursive_flag = 0;

    while ((opt = getopt(argc, argv, "lRx")) != -1) {
        switch (opt) {
            case 'l': long_format = 1; break;
            case 'x': column_mode = 1; break;
            case 'R': recursive_flag = 1; break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [-R] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    const char *target_dir = (optind < argc) ? argv[optind] : ".";
    do_ls(target_dir, long_format, column_mode, recursive_flag);
    return 0;
}

