#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>


// Function prototypes
void display_long_listing(char **files, int count);
void display_columnar(char **files, int count);
void display_horizontal(char **files, int count);
int compare_filenames(const void *a, const void *b);

int main(int argc, char *argv[]) {
    const char *path = ".";
    int long_listing = 0, horizontal = 0;

    // Parse command line options
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0)
            long_listing = 1;
        else if (strcmp(argv[i], "-x") == 0)
            horizontal = 1;
        else
            path = argv[i];
    }

    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return EXIT_FAILURE;
    }

    // Read all entries dynamically
    struct dirent *entry;
    char **filenames = NULL;
    int count = 0;
    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files (unless implementing -a later)
        if (entry->d_name[0] == '.')
            continue;
        filenames = realloc(filenames, (count + 1) * sizeof(char *));
        filenames[count] = strdup(entry->d_name);
        count++;
    }
    closedir(dir);

    // Sort filenames alphabetically
    qsort(filenames, count, sizeof(char *), compare_filenames);

    // Display according to mode
    if (long_listing)
        display_long_listing(filenames, count);
    else if (horizontal)
        display_horizontal(filenames, count);
    else
        display_columnar(filenames, count);

    // Free memory
    for (int i = 0; i < count; i++)
        free(filenames[i]);
    free(filenames);

    return 0;
}

// Comparison function for qsort
int compare_filenames(const void *a, const void *b) {
    const char *fa = *(const char **)a;
    const char *fb = *(const char **)b;
    return strcmp(fa, fb);
}

// Long listing (-l)
void display_long_listing(char **files, int count) {
    struct stat st;
    for (int i = 0; i < count; i++) {
        if (stat(files[i], &st) == 0)
            printf("%-10ld %s\n", (long)st.st_size, files[i]);
        else
            perror(files[i]);
    }
}

// Default column display
void display_columnar(char **files, int count) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int maxlen = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(files[i]);
        if (len > maxlen)
            maxlen = len;
    }

    int cols = w.ws_col / (maxlen + 2);
    if (cols < 1)
        cols = 1;
    int rows = (count + cols - 1) / cols;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int idx = c * rows + r;
            if (idx < count)
                printf("%-*s", maxlen + 2, files[idx]);
        }
        printf("\n");
    }
}

// Horizontal display (-x)
void display_horizontal(char **files, int count) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int maxlen = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(files[i]);
        if (len > maxlen)
            maxlen = len;
    }

    int cols = w.ws_col / (maxlen + 2);
    if (cols < 1)
        cols = 1;

    for (int i = 0; i < count; i++) {
        printf("%-*s", maxlen + 2, files[i]);
        if ((i + 1) % cols == 0)
            printf("\n");
    }
    printf("\n");
}

