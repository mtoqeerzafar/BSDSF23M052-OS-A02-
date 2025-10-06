/*
 * Programming Assignment 02: ls-v1.1.0
 * Feature: Long Listing Format (-l)
 * Author: mtoqeerzafar
 * Version: 1.1.0
 * Description:
 * This version of ls adds support for the -l option that prints
 * detailed file information (permissions, owner, group, size, time).
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <getopt.h>

extern int errno;

void do_ls(const char *dir, int long_format);
void show_file_info(const char *filename, const struct stat *info);
void mode_to_letters(int mode, char str[]);

int main(int argc, char *argv[])
{
    int opt;
    int long_format = 0; // flag for -l

    // Parse command-line options (-l)
    while ((opt = getopt(argc, argv, "l")) != -1)
    {
        switch (opt)
        {
        case 'l':
            long_format = 1;
            break;
        default:
            fprintf(stderr, "Usage: %s [-l] [directory...]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // If no directory is given, list current directory
    if (optind == argc)
    {
        do_ls(".", long_format);
    }
    else
    {
        // For each directory provided
        for (int i = optind; i < argc; i++)
        {
            printf("Directory listing of %s:\n", argv[i]);
            do_ls(argv[i], long_format);
            puts("");
        }
    }

    return 0;
}

void do_ls(const char *dir, int long_format)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    errno = 0;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue; // skip hidden files

        if (long_format)
        {
            struct stat info;
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

            if (stat(path, &info) == -1)
            {
                perror("stat");
                continue;
            }
            show_file_info(entry->d_name, &info);
        }
        else
        {
            printf("%s\n", entry->d_name);
        }
    }

    if (errno != 0)
        perror("readdir failed");

    closedir(dp);
}

// Print detailed info for each file (like ls -l)
void show_file_info(const char *filename, const struct stat *info)
{
    char mode_str[11];
    mode_to_letters(info->st_mode, mode_str);

    struct passwd *pw = getpwuid(info->st_uid);
    struct group *gr = getgrgid(info->st_gid);

    char time_str[32];
    strncpy(time_str, ctime(&info->st_mtime), sizeof(time_str));
    time_str[strlen(time_str) - 1] = '\0'; // remove newline

    printf("%s %3ld %-8s %-8s %8ld %s %s\n",
           mode_str,
           (long)info->st_nlink,
           pw ? pw->pw_name : "unknown",
           gr ? gr->gr_name : "unknown",
           (long)info->st_size,
           time_str,
           filename);
}

// Convert st_mode integer into string like "drwxr-xr-x"
void mode_to_letters(int mode, char str[])
{
    strcpy(str, "----------");

    if (S_ISDIR(mode))
        str[0] = 'd';
    if (S_ISCHR(mode))
        str[0] = 'c';
    if (S_ISBLK(mode))
        str[0] = 'b';

    if (mode & S_IRUSR)
        str[1] = 'r';
    if (mode & S_IWUSR)
        str[2] = 'w';
    if (mode & S_IXUSR)
        str[3] = 'x';

    if (mode & S_IRGRP)
        str[4] = 'r';
    if (mode & S_IWGRP)
        str[5] = 'w';
    if (mode & S_IXGRP)
        str[6] = 'x';

    if (mode & S_IROTH)
        str[7] = 'r';
    if (mode & S_IWOTH)
        str[8] = 'w';
    if (mode & S_IXOTH)
        str[9] = 'x';
}

