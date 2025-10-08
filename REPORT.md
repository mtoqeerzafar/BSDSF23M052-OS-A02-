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

Hence, ioctl provides a dynamic and user-friendly display layout


Feature 4 :

Q1: General logic for printing items in a “down then across” columnar format

When displaying filenames “down then across,” the program fills each column vertically first before moving to the next column.
A single linear loop cannot achieve this because it lists items row by row (horizontally).
To print vertically, you must calculate:

the number of rows (num_rows = ceil(total_files / num_columns)),

and use nested indexing to access items in the order of columns.

Example logic:

for (row = 0; row < num_rows; row++) {
    for (col = 0; col < num_cols; col++) {
        index = row + col * num_rows;
        if (index < total_files)
            printf("%-*s", column_width, filenames[index]);
    }
    printf("\n");
}


This ensures items appear top-to-bottom, then left-to-right.

Q2: Purpose of the ioctl system call

ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) is used to dynamically get the terminal’s current width and height in characters.
This allows the program to:

Adjust the number of columns that fit on the screen.

Prevent text wrapping or misalignment when resizing the terminal.

If only a fixed width (like 80 columns) were used:

The output could appear misaligned or overflow on smaller screens.

The display would waste space or look compressed on larger terminals.
Dynamic detection ensures a responsive layout across environments.




feature 6 :

Feature-6: ls-v1.5.0 – Colorized Output Based on File Type
1. How do ANSI escape codes work to produce color in a standard Linux terminal? Show the specific code sequence for printing text in green.

ANSI escape codes are special character sequences recognized by most terminal emulators to control formatting such as color, boldness, or cursor movement.
Each escape code begins with the ESC character (\033) followed by a series of parameters ending with an “m”, which signals the terminal to change text attributes.

Example structure:

\033[<style>;<foreground>;<background>m


\033[ → Escape sequence start

<style> → Text style (e.g., 0 = normal, 1 = bold)

<foreground> → Text color (e.g., 32 = green)

<background> → Optional background color

m → Terminates the escape code

To print text in green:

printf("\033[0;32mHello\033[0m\n");


Explanation:

\033[0;32m → Start green color

Hello → Text in green

\033[0m → Reset to default terminal color

2. To color an executable file, you need to check its permission bits. Explain which bits in the st_mode field you need to check to determine if a file is executable by the owner, group, or others.

The st_mode field from the stat structure contains both the file type and permission bits.
To check if a file is executable, we examine the execute permission bits using bitwise AND operations:

Permission	Macro	Bitmask	Meaning
Owner execute	S_IXUSR	0100	Owner can execute
Group execute	S_IXGRP	0010	Group can execute
Others execute	S_IXOTH	0001	Others can execute

Example in C:

struct stat fileStat;
stat(filename, &fileStat);

if (fileStat.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
    // File is executable → print in green
}



Feature 7:


Feature-7: ls-v1.6.0 – Recursive Listing (-R Option)
1. In a recursive function, what is a "base case"? In the context of your recursive ls, what is the base case that stops the recursion from continuing forever?

In recursion, a base case is the condition that terminates further recursive calls.
Without it, the function would call itself indefinitely, leading to infinite recursion and a crash.

In the recursive ls implementation, the base case is when:

The function encounters a non-directory file, or

It finds the special directories . (current) or .. (parent).

These cases stop further recursion, preventing the function from re-entering the same directories repeatedly.

Example:

if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
    return;  // base case

2. Explain why it is essential to construct a full path (e.g., "parent_dir/subdir") before making a recursive call. What would happen if you simply called do_ls("subdir") from within the do_ls("parent_dir") function call?

It’s essential to construct the full path (like "parent_dir/subdir") because the program changes directories logically, not physically.
Without the full path, the recursive call do_ls("subdir") would always try to open "subdir" relative to the current working directory, not relative to "parent_dir".

If "subdir" exists only inside "parent_dir", this would result in:

opendir("subdir") failing with "No such file or directory",

Incorrect directory traversal, or

Missing recursive listings.

Thus, constructing the absolute or combined path ensures the recursive traversal moves correctly down the directory hierarchy.

Example:

snprintf(fullpath, sizeof(fullpath), "%s/%s", parent, child);
do_ls(fullpath, mode);


✅ Summary

Added support for -R (recursive listing).

Implemented recursive directory traversal with safe path concatenation.

Prevented infinite recursion via base cases for . and ...

Integrated with colorized display from previous versions.

Clean memory management for dynamically allocated filename arrays.


feature 7:

[200~1. In a recursive function, what is a "base case"?

A base case is the condition in a recursive function that stops further recursive calls.
Without a base case, the function would keep calling itself indefinitely, leading to a stack overflow error.

In the context of the recursive ls command:
The base case occurs when the function reaches a directory that has no subdirectories or when an entry is not a directory (e.g., a regular file).
At that point, the recursion stops — the function lists the current directory’s contents and then returns.

Example (simplified logic):

if (!S_ISDIR(st.st_mode))
    return;   // Base case — stop recursion for non-directory files

2. Why is it essential to construct a full path (e.g., "parent_dir/subdir") before making a recursive call?

It is essential to build the full path because the recursive call needs to know the exact location of the subdirectory in the filesystem.
If you only passed "subdir" instead of "parent_dir/subdir", the program would incorrectly look for "subdir" in the current working directory, not inside its parent directory.

For example:

✅ Correct: do_ls("src/utils") — looks inside src/utils

❌ Wrong: do_ls("utils") — looks for utils in the current directory, not in src

Result if full path is not used:

The recursion would fail to find the correct subdirectory.

It could repeatedly scan the same directories, leading to incorrect or infinite recursion.

Hence: Constructing the full path ensures that each recursive call correctly points to the subdirectory being processed.
