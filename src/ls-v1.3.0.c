/*
 * Programming Assignment 02: ls-v1.3.0
 * Version 1.3.0 – Horizontal Column Display (-x) + Column Display (down-then-across) + -l long listing
 * Author: mtoqeerzafar
 *
 * Usage:
 *   ./bin/ls-v1.3.0           -> default: down-then-across columns
 *   ./bin/ls-v1.3.0 -l        -> long listing (like ls -l)
 *   ./bin/ls-v1.3.0 -x        -> horizontal (across-then-down) columns
 *   ./bin/ls-v1.3.0 -l -x     -> -l takes precedence (long listing)
 *
 * Notes:
 * - Hidden files (starting with '.') are skipped.
 * - Filenames are sorted alphabetically before display.
 * - Uses ioctl(TIOCGWINSZ) to detect terminal width; fallback to 80 columns.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // getopt
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/ioctl.h>   // ioctl, winsize
#include <errno.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* Prototypes */
void do_ls(const char *dir, int long_format, int horizontal);
void print_long_format(const char *dir, const char *name);
void print_columns(char **names, size_t count, int term_width);
void print_horizontal(char **names, size_t count, int term_width);

/* Utility: join path */
static void join_path(const char *dir, const char *name, char *out, size_t out_sz) {
    if (!dir || dir[0] == '\0' || (dir[0]=='.' && dir[1]=='\0')) {
        snprintf(out, out_sz, "%s", name);
    } else {
        size_t len = strlen(dir);
        if (dir[len-1] == '/')
            snprintf(out, out_sz, "%s%s", dir, name);
        else
            snprintf(out, out_sz, "%s/%s", dir, name);
    }
}

/* Compare for qsort */
static int name_cmp(const void *a, const void *b) {
    const char * const *pa = a;
    const char * const *pb = b;
    return strcmp(*pa, *pb);
}

/* Return terminal width or fallback 80 */
static int get_terminal_width(void) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
        return (int)w.ws_col;
    return 80;
}

/* --- Main --- */
int main(int argc, char *argv[]) {
    int opt;
    int long_format = 0;
    int horizontal = 0;

    /* Parse options: -l (long), -x (horizontal) */
    while ((opt = getopt(argc, argv, "lx")) != -1) {
        switch (opt) {
            case 'l':
                long_format = 1;
                break;
            case 'x':
                horizontal = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [directory...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    /* If both -l and -x: -l takes precedence (long listing) */
    if (long_format) horizontal = 0;

    if (optind == argc) {
        do_ls(".", long_format, horizontal);
    } else {
        for (int i = optind; i < argc; ++i) {
            if (argc - optind > 1)
                printf("%s:\n", argv[i]);
            do_ls(argv[i], long_format, horizontal);
            if (i + 1 < argc) putchar('\n');
        }
    }

    return 0;
}

/* --- do_ls: read directory entries into an array, sort, then dispatch to a printing function --- */
void do_ls(const char *dir, int long_format, int horizontal) {
    DIR *dp = opendir(dir && dir[0] ? dir : ".");
    if (!dp) {
        fprintf(stderr, "Cannot open directory '%s': %s\n", dir ? dir : ".", strerror(errno));
        return;
    }

    char **names = NULL;
    size_t count = 0;
    size_t cap = 16;
    names = malloc(cap * sizeof(char *));
    if (!names) {
        perror("malloc");
        closedir(dp);
        return;
    }

    struct dirent *entry;
    errno = 0;
    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] == '.') continue;  /* skip hidden */

        if (count >= cap) {
            size_t ncap = cap * 2;
            char **tmp = realloc(names, ncap * sizeof(char *));
            if (!tmp) {
                perror("realloc");
                for (size_t i = 0; i < count; ++i) free(names[i]);
                free(names);
                closedir(dp);
                return;
            }
            names = tmp;
            cap = ncap;
        }
        names[count] = strdup(entry->d_name);
        if (!names[count]) {
            perror("strdup");
            for (size_t i = 0; i < count; ++i) free(names[i]);
            free(names);
            closedir(dp);
            return;
        }
        ++count;
    }

    if (errno != 0) perror("readdir");

    closedir(dp);

    if (count == 0) {
        free(names);
        return;
    }

    /* Sort names alphabetically */
    qsort(names, count, sizeof(char *), name_cmp);

    /* Get terminal width once */
    int term_width = get_terminal_width();

    /* Dispatch */
    if (long_format) {
        for (size_t i = 0; i < count; ++i)
            print_long_format(dir, names[i]);
    } else if (horizontal) {
        print_horizontal(names, count, term_width);
    } else {
        print_columns(names, count, term_width);
    }

    /* cleanup */
    for (size_t i = 0; i < count; ++i) free(names[i]);
    free(names);
}

/* --- print_long_format: prints one long listing entry like 'ls -l' --- */
void print_long_format(const char *dir, const char *name) {
    char path[PATH_MAX];
    join_path(dir, name, path, sizeof(path));

    struct stat st;
    if (lstat(path, &st) == -1) {
        fprintf(stderr, "lstat %s: %s\n", path, strerror(errno));
        return;
    }

    /* file type */
    char ft = '-';
    if (S_ISDIR(st.st_mode)) ft = 'd';
    else if (S_ISLNK(st.st_mode)) ft = 'l';
    else if (S_ISCHR(st.st_mode)) ft = 'c';
    else if (S_ISBLK(st.st_mode)) ft = 'b';
    else if (S_ISFIFO(st.st_mode)) ft = 'p';
    else if (S_ISSOCK(st.st_mode)) ft = 's';

    /* permissions */
    char perms[11];
    perms[0] = ft;
    perms[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    perms[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    perms[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    perms[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    perms[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
    perms[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';
    perms[10] = '\0';

    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);

    char timebuf[64];
    struct tm *tm = localtime(&st.st_mtime);
    if (tm) strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);
    else strcpy(timebuf, "??? ???");

    printf("%s %3ld %-8s %-8s %8ld %s %s\n",
           perms,
           (long)st.st_nlink,
           pw ? pw->pw_name : "?",
           gr ? gr->gr_name : "?",
           (long)st.st_size,
           timebuf,
           name);
}

/* --- print_columns: down-then-across (column-major) --- */
void print_columns(char **names, size_t count, int term_width) {
    if (count == 0) return;

    /* find longest name */
    size_t maxlen = 0;
    for (size_t i = 0; i < count; ++i) {
        size_t l = strlen(names[i]);
        if (l > maxlen) maxlen = l;
    }

    int spacing = 2;
    size_t col_width = maxlen + spacing;
    if (col_width == 0) col_width = 1;

    int num_cols = (int)(term_width / (int)col_width);
    if (num_cols < 1) num_cols = 1;

    int num_rows = (int)((count + num_cols - 1) / num_cols); /* ceil */

    for (int r = 0; r < num_rows; ++r) {
        for (int c = 0; c < num_cols; ++c) {
            int idx = c * num_rows + r;
            if ((size_t)idx < count) {
                printf("%-*s", (int)col_width, names[idx]);
            }
        }
        putchar('\n');
    }
}

/* --- print_horizontal: across-then-down (row-major) --- */
void print_horizontal(char **names, size_t count, int term_width) {
    if (count == 0) return;

    /* find longest name */
    size_t maxlen = 0;
    for (size_t i = 0; i < count; ++i) {
        size_t l = strlen(names[i]);
        if (l > maxlen) maxlen = l;
    }

    int spacing = 2;
    size_t col_width = maxlen + spacing;
    if (col_width == 0) col_width = 1;

    int current_width = 0;
    for (size_t i = 0; i < count; ++i) {
        /* If single filename wider than term, still print it on its own line */
        if ((size_t)term_width < col_width) {
            /* print name and newline */
            printf("%s\n", names[i]);
            current_width = 0;
            continue;
        }

        if (current_width + (int)col_width > term_width) {
            putchar('\n');
            current_width = 0;
        }
        printf("%-*s", (int)col_width, names[i]);
        current_width += (int)col_width;
    }
    putchar('\n');
}

