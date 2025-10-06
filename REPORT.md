# Feature 1: Project Setup and Initial Build

## Tasks Summary

### Task 1: Create and Clone Repo
Created GitHub repo `BSDSF23M052-OS-A02`, cloned to local system.

### Task 2: Populate Starter Code
Copied `ls-v1.0.0.c` and `Makefile` from instructor's provided resources.

### Task 3: Project Structure
Created directories: `bin/`, `obj/`, `man/`, and file `REPORT.md`.

### Task 4: Build and Test
Used `make` command to compile source. Binary `ls` generated in `bin/` and verified working.

### Task 5: Commit and Push
Committed all files and pushed to GitHub using:

Feature 2:

1. Difference between stat() and lstat()

The crucial difference between stat() and lstat() lies in how they handle symbolic links:

stat() follows the symbolic link and returns information about the target file that the link points to.

lstat() returns information about the link itself, not the target.

When to use lstat() in the context of the ls command

In the implementation of the ls command:

If you are listing files and want to show details about the symbolic link itself (for example, its permissions, size, and the fact that it’s a link), then lstat() is more appropriate.

stat() would instead show information about the file being pointed to, which is not what ls -l typically does when listing links.

✅ Example:
If /tmp/link -> /etc/passwd

stat("/tmp/link") → returns info about /etc/passwd

lstat("/tmp/link") → returns info about /tmp/link (the symbolic link)

2. Using bitwise operators and macros to extract file type and permissions

The st_mode field in struct stat is an integer that encodes both:

File type (e.g., directory, regular file, symbolic link)

File permissions (e.g., read, write, execute for owner/group/others)

We can extract this information using bitwise operators (&) and predefined macros.

(a) Extracting file type

File types are identified using the S_IFMT bitmask along with macros like S_IFDIR, S_IFREG, S_IFLNK, etc.

struct stat sb;

lstat("file.txt", &sb);

if ((sb.st_mode & S_IFMT) == S_IFDIR)
    printf("It’s a directory.\n");
else if ((sb.st_mode & S_IFMT) == S_IFREG)
    printf("It’s a regular file.\n");
else if ((sb.st_mode & S_IFMT) == S_IFLNK)
    printf("It’s a symbolic link.\n");


Here:

S_IFMT masks the file-type bits.

The result is compared to specific macros like S_IFDIR or S_IFREG.

(b) Extracting file permissions

Permission bits can be checked using macros like S_IRUSR, S_IWUSR, S_IXUSR, etc.

if (sb.st_mode & S_IRUSR)
    printf("Owner has read permission.\n");
if (sb.st_mode & S_IWUSR)
    printf("Owner has write permission.\n");
if (sb.st_mode & S_IXUSR)
    printf("Owner has execute permission.\n");


Here:

S_IRUSR = Read permission for owner

S_IWUSR = Write permission for owner

S_IXUSR = Execute permission for owner

The & (bitwise AND) operator isolates specific bits to test if they’re set.


| Purpose          | Macro Example                   | Description                    |
| ---------------- | ------------------------------- | ------------------------------ |
| File type        | `S_IFDIR`, `S_IFREG`, `S_IFLNK` | Determine type of file         |
| Mask type bits   | `S_IFMT`                        | Used to isolate file-type bits |
| Permission bits  | `S_IRUSR`, `S_IWUSR`, `S_IXUSR` | Owner permissions              |
| Bitwise operator | `&`                             | Tests if a specific bit is set |




Feature 3:

[200~REPORT.md Answers
Q1: Explain the general logic for printing items in a “down then across” columnar format. Why is a simple single loop through the list of filenames insufficient for this task?

Answer:
The "down-then-across" column format means filenames are printed top-to-bottom in each column before moving horizontally to the next column.
For example:

A   D   G
B   E   H
C   F


To achieve this, we first determine:

The total number of filenames (n)

The number of columns that can fit based on terminal width

The number of rows needed per column

We then print items using nested loops:

The outer loop runs through rows

The inner loop indexes each column’s element by (col * num_rows + row)

A single loop is insufficient because filenames must be rearranged by their position in a grid — not printed linearly. A one-dimensional iteration would only print items across a single row, not in a grid aligned by columns.

Q2: What is the purpose of the ioctl system call in this context? What would be the limitations of your program if you only used a fixed-width fallback (e.g., 80 columns) instead of detecting the terminal size?

Answer:
The ioctl system call is used to query terminal properties dynamically — specifically, to obtain the current width (number of columns) of the terminal window.
This allows the program to adjust the number of filenames displayed per row according to the actual screen size.

If we only used a fixed-width fallback (like 80 columns):

The display would not adapt to different terminal sizes.

On narrow screens, filenames would wrap awkwardly or overlap.

On wide screens, excessive empty space would appear and fewer columns would be used than possible.

It would reduce usability and responsiveness — the output would not resemble real ls behavior.

Hence, ioctl provides a dynamic and user-friendly display layout.
