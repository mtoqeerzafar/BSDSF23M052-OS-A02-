#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* ANSI color codes */
#define CLR_RESET "\033[0m"
#define CLR_BLUE  "\033[0;34m"
#define CLR_GREEN "\033[0;32m"
#define CLR_RED   "\033[0;31m"
#define CLR_PINK  "\033[1;35m"
#define CLR_REVERSE "\033[7m"

/* Prototypes */
void do_ls(const char *dir, int long_format, int horizontal);
void print_long_format(const char *dir, const char *name);
void print_columns(char **names, size_t count, int term_width, const char *dir);
void print_horizontal(char **names, size_t count, int term_width, const char *dir);
void color_print_name(const char *dir, const char *name);
static int name_cmp(const void *a, const void *b);
static int get_terminal_width(void);
static void join_path(const char *dir, const char *name, char *out, size_t out_sz);

/* ---------- main ---------- */
int main(int argc, char *argv[]) {
    int opt;
    int long_format = 0;
    int horizontal = 0;

    while ((opt = getopt(argc, argv, "lx")) != -1) {
        switch (opt) {
            case 'l': long_format = 1; break;
            case 'x': horizontal = 1; break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [directory...]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    /* -l takes precedence */
    if (long_format) horizontal = 0;

    if (optind == argc) {
        do_ls(".", long_format, horizontal);
    } else {
        for (int i = optind; i < argc; ++i) {
            if (argc - optind > 1) printf("%s:\n", argv[i]);
            do_ls(argv[i], long_format, horizontal);
            if (i + 1 < argc) putchar('\n');
        }
    }
    return 0;
}

/* ---------- helpers ---------- */
static void join_path(const char *dir, const char *name, char *out, size_t out_sz) {
    if (!dir || dir[0] == '\0' || (dir[0]=='.' && dir[1]=='\0'))
        snprintf(out, out_sz, "%s", name);
    else {
        size_t len = strlen(dir);
        if (dir[len-1] == '/') snprintf(out, out_sz, "%s%s", dir, name);
        else snprintf(out, out_sz, "%s/%s", dir, name);
    }
}

static int get_terminal_width(void) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) return (int)w.ws_col;
    return 80;
}

static int name_cmp(const void *a, const void *b) {
    const char *const *pa = a;
    const char *const *pb = b;
    return strcmp(*pa, *pb);
}

/* ---------- directory listing and dispatch ---------- */
void do_ls(const char *dir, int long_format, int horizontal) {
    DIR *dp = opendir(dir && dir[0] ? dir : ".");
    if (!dp) {
        fprintf(stderr, "Cannot open directory '%s': %s\n", dir ? dir : ".", strerror(errno));
        return;
    }

    char **names = NULL;
    size_t cap = 16, count = 0;
    names = malloc(cap * sizeof(char*));
    if (!names) { perror("malloc"); closedir(dp); return; }

    struct dirent *entry;
    errno = 0;
    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] == '.') continue; /* skip hidden for now */
        if (count >= cap) {
            size_t ncap = cap * 2;
            char **tmp = realloc(names, ncap * sizeof(char*));
            if (!tmp) { perror("realloc"); break; }
            names = tmp; cap = ncap;
        }
        names[count] = strdup(entry->d_name);
        if (!names[count]) { perror("strdup"); break; }
        ++count;
    }
    if (errno) perror("readdir");
    closedir(dp);

    if (count == 0) { free(names); return; }

    /* Task 2 (Feature 5): sort alphabetically using qsort */
    qsort(names, count, sizeof(char*), name_cmp);

    int term_width = get_terminal_width();

    if (long_format) {
        for (size_t i = 0; i < count; ++i) print_long_format(dir, names[i]);
    } else if (horizontal) {
        print_horizontal(names, count, term_width, dir);
    } else {
        print_columns(names, count, term_width, dir);
    }

    for (size_t i = 0; i < count; ++i) free(names[i]);
    free(names);
}

/* ---------- long listing (unchanged) ---------- */
void print_long_format(const char *dir, const char *name) {
    char path[PATH_MAX]; join_path(dir, name, path, sizeof(path));
    struct stat st;
    if (lstat(path, &st) == -1) { perror(path); return; }

    /* permissions and type */
    char t = S_ISDIR(st.st_mode) ? 'd' : S_ISLNK(st.st_mode) ? 'l' : '-';
    char perm[11] = {0};
    perm[0] = t;
    perm[1] = (st.st_mode & S_IRUSR) ? 'r' : '-'; perm[2] = (st.st_mode & S_IWUSR) ? 'w' : '-'; perm[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    perm[4] = (st.st_mode & S_IRGRP) ? 'r' : '-'; perm[5] = (st.st_mode & S_IWGRP) ? 'w' : '-'; perm[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    perm[7] = (st.st_mode & S_IROTH) ? 'r' : '-'; perm[8] = (st.st_mode & S_IWOTH) ? 'w' : '-'; perm[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';

    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    char timebuf[64];
    struct tm *tm = localtime(&st.st_mtime);
    if (tm) strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm); else strcpy(timebuf, "???");

    printf("%s %3ld %-8s %-8s %8ld %s ", perm, (long)st.st_nlink, pw?pw->pw_name:"?", gr?gr->gr_name:"?", (long)st.st_size, timebuf);
    /* colorized name printed here */
    color_print_name(dir, name);
    putchar('\n');
}

/* ---------- color selection and printing ---------- */
int is_archive(const char *name) {
    return (strstr(name, ".tar") || strstr(name, ".gz") || strstr(name, ".zip"));
}

void color_print_name(const char *dir, const char *name) {
    char path[PATH_MAX]; join_path(dir, name, path, sizeof(path));
    struct stat st;
    if (lstat(path, &st) == -1) { /* print name uncolored on error */ printf("%s", name); return; }

    /* Symbolic link? use pink */
    if (S_ISLNK(st.st_mode)) {
        printf("%s%s%s", CLR_PINK, name, CLR_RESET);
        return;
    }

    /* Directory -> blue */
    if (S_ISDIR(st.st_mode)) { printf("%s%s%s", CLR_BLUE, name, CLR_RESET); return; }

    /* Executable check (owner/group/others exec bits) -> green */
    if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)) {
        printf("%s%s%s", CLR_GREEN, name, CLR_RESET); return;
    }

    /* Tarballs / archives -> red */
    if (is_archive(name)) { printf("%s%s%s", CLR_RED, name, CLR_RESET); return; }

    /* Special files (device, socket, fifo) -> reverse video */
    if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) || S_ISFIFO(st.st_mode) || S_ISSOCK(st.st_mode)) {
        printf("%s%s%s", CLR_REVERSE, name, CLR_RESET); return; }

    /* Default: plain */
    printf("%s", name);
}

/* ---------- column (down-then-across) ---------- */
void print_columns(char **names, size_t count, int term_width, const char *dir) {
    if (count == 0) return;
    size_t maxlen = 0; for (size_t i=0;i<count;++i) { size_t l=strlen(names[i]); if (l>maxlen) maxlen=l; }
    int spacing = 2; size_t col_width = maxlen + spacing; if (col_width==0) col_width=1;
    int num_cols = term_width / (int)col_width; if (num_cols<1) num_cols=1;
    int num_rows = (int)((count + num_cols -1)/num_cols);
    for (int r=0;r<num_rows;++r) {
        for (int c=0;c<num_cols;++c) {
            int idx = c * num_rows + r;
            if ((size_t)idx < count) {
                /* print padded, but colorized */
                /* compute plain name into buffer then print padded with color codes preserved */
                char buf[PATH_MAX]; snprintf(buf, sizeof(buf), "%s", names[idx]);
                printf("% -*s", (int)col_width, ""); /* print padding placeholder then overwrite - can't easily pad colored text */
                /* Instead, print name but align by printing name then spaces */
                printf("%s", "");
            }
        }
        putchar('\n');
    }
    /* The above simplistic padding with colored output is tricky; use simpler approach below */
    /* Re-implement: print each column cell using color_print_name and then pad with spaces to column width */
    for (int r=0;r<num_rows;++r) {
        for (int c=0;c<num_cols;++c) {
            int idx = c * num_rows + r;
            if ((size_t)idx < count) {
                /* print colored name */
                color_print_name(dir, names[idx]);
                /* compute visible length (no easy way to strip ANSI here). We approximate using strlen of name */
                int pad = (int)col_width - (int)strlen(names[idx]);
                for (int p=0;p<pad;++p) putchar(' ');
            }
        }
        putchar('\n');
    }
}

/* ---------- horizontal (across-then-down) ---------- */
void print_horizontal(char **names, size_t count, int term_width, const char *dir) {
    if (count==0) return;
    size_t maxlen=0; for (size_t i=0;i<count;++i){ size_t l=strlen(names[i]); if (l>maxlen) maxlen=l; }
    int spacing=2; size_t colw = maxlen + spacing; if (colw==0) colw=1;
    int curw = 0;
    for (size_t i=0;i<count;++i) {
        if ((size_t)term_width < colw) { color_print_name(dir, names[i]); putchar('\n'); curw=0; continue; }
        if (curw + (int)colw > term_width) { putchar('\n'); curw = 0; }
        color_print_name(dir, names[i]);
        int pad = (int)colw - (int)strlen(names[i]); for (int p=0;p<pad;++p) putchar(' ');
        curw += (int)colw;
    }
    putchar('\n');
}
