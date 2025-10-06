/*
 * Programming Assignment 02: ls-v1.2.0
 * Version 1.2.0 – Column Display (Down Then Across)
 * Features:
 *   - Default: multi-column display (down then across)
 *   - With -l flag: long listing format (permissions, owner, size, time)
 */

#define _GNU_SOURCE     // enable GNU extensions for strdup and getopt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // for getopt, lstat
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/ioctl.h>   // for ioctl, TIOCGWINSZ
#include <errno.h>


extern int errno;

void do_ls(const char *dir, int long_format);
void print_long_format(const char *path, const char *filename);
void print_columns(char **names, int count);

int main(int argc, char *argv[]) {
    int opt;
    int long_format = 0;

    // Detect -l flag
    while ((opt = getopt(argc, argv, "l")) != -1) {
        if (opt == 'l')
            long_format = 1;
        else {
            fprintf(stderr, "Usage: %s [-l] [directory...]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // If no directory given, use current directory
    if (optind == argc) {
        do_ls(".", long_format);
    } else {
        for (int i = optind; i < argc; i++) {
            printf("Directory listing of %s:\n", argv[i]);
            do_ls(argv[i], long_format);
            puts("");
        }
    }

    return 0;
}

/* ---------- Core Listing Logic ---------- */
void do_ls(const char *dir, int long_format) {
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (!dp) {
        perror("Cannot open directory");
        return;
    }

    if (long_format) {
        while ((entry = readdir(dp)) != NULL) {
            if (entry->d_name[0] == '.')
                continue;
            print_long_format(dir, entry->d_name);
        }
    } else {
        // --- Collect filenames first ---
        char **names = NULL;
        int count = 0;
        int capacity = 16;
        names = malloc(capacity * sizeof(char *));
        if (!names) {
            perror("malloc");
            closedir(dp);
            return;
        }

        while ((entry = readdir(dp)) != NULL) {
            if (entry->d_name[0] == '.')
                continue;
            if (count >= capacity) {
                capacity *= 2;
                names = realloc(names, capacity * sizeof(char *));
                if (!names) {
                    perror("realloc");
                    closedir(dp);
                    return;
                }
            }
            names[count++] = strdup(entry->d_name);
        }
        closedir(dp);

        // Sort alphabetically
        for (int i = 0; i < count - 1; i++) {
            for (int j = i + 1; j < count; j++) {
                if (strcmp(names[i], names[j]) > 0) {
                    char *tmp = names[i];
                    names[i] = names[j];
                    names[j] = tmp;
                }
            }
        }

        print_columns(names, count);

        for (int i = 0; i < count; i++)
            free(names[i]);
        free(names);
    }
}

/* ---------- Long Listing Format ---------- */
void print_long_format(const char *path, const char *filename) {
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, filename);

    struct stat st;
    if (lstat(fullpath, &st) == -1) {
        perror("lstat");
        return;
    }

    // File type
    printf( S_ISDIR(st.st_mode) ? "d" :
            S_ISLNK(st.st_mode) ? "l" :
            S_ISCHR(st.st_mode) ? "c" :
            S_ISBLK(st.st_mode) ? "b" :
            S_ISFIFO(st.st_mode)? "p" :
            S_ISSOCK(st.st_mode)? "s" : "-");

    // Permissions
    printf("%c", (st.st_mode & S_IRUSR) ? 'r' : '-');
    printf("%c", (st.st_mode & S_IWUSR) ? 'w' : '-');
    printf("%c", (st.st_mode & S_IXUSR) ? 'x' : '-');
    printf("%c", (st.st_mode & S_IRGRP) ? 'r' : '-');
    printf("%c", (st.st_mode & S_IWGRP) ? 'w' : '-');
    printf("%c", (st.st_mode & S_IXGRP) ? 'x' : '-');
    printf("%c", (st.st_mode & S_IROTH) ? 'r' : '-');
    printf("%c", (st.st_mode & S_IWOTH) ? 'w' : '-');
    printf("%c", (st.st_mode & S_IXOTH) ? 'x' : '-');

    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", localtime(&st.st_mtime));

    printf(" %3ld %-8s %-8s %8ld %s %s\n",
           st.st_nlink,
           pw ? pw->pw_name : "???",
           gr ? gr->gr_name : "???",
           (long) st.st_size,
           timebuf,
           filename);
}

/* ---------- Multi-Column Display ---------- */
void print_columns(char **names, int count) {
    if (count == 0) return;

    // Get terminal width
    struct winsize w;
    int term_width = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
        term_width = w.ws_col;

    // Find longest filename
    int maxlen = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(names[i]);
        if (len > maxlen)
            maxlen = len;
    }

    int spacing = 2;
    int col_width = maxlen + spacing;
    int num_cols = term_width / col_width;
    if (num_cols < 1) num_cols = 1;
    int num_rows = (count + num_cols - 1) / num_cols;

    // Print down-then-across
    for (int r = 0; r < num_rows; r++) {
        for (int c = 0; c < num_cols; c++) {
            int index = c * num_rows + r;
            if (index < count)
                printf("%-*s", col_width, names[index]);
        }
        printf("\n");
    }
}

