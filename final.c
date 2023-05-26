#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

// Function declarations
void handle_regular_file(char *path);
void handle_directory(char *path);
void handle_symlink(char *path);
void handle_child_process(char *path);
void print_access_rights(struct stat file_stat);
void execute_script(char *script_path, char *c_file_path);
int count_lines(char *file_path);
void print_file_size(char *path);
void print_hard_link_count(char *path);
void print_last_modification_time(char *path);
void create_symbolic_link(char *path, char *link_path);

int main(int argc, char *argv[]) {
    pid_t pid;

    if(argc < 2) {
        printf("No file or directory provided. Please provide file or directory paths as arguments.\n");
        return 1;
    }

    for(int i = 1; i < argc; i++) {
        pid = fork();

        if(pid < 0) {
            printf("Failed to fork process.\n");
            return 1;
        } else if(pid == 0) {
            // In child process
            struct stat path_stat;
            stat(argv[i], &path_stat);

            if(S_ISREG(path_stat.st_mode)) {
                handle_regular_file(argv[i]);
            } else if(S_ISDIR(path_stat.st_mode)) {
                handle_directory(argv[i]);
            } else if(S_ISLNK(path_stat.st_mode)) {
                handle_symlink(argv[i]);
            } else {
                printf("Unsupported file type.\n");
                exit(1);
            }

            // Create another child process for additional tasks
            pid_t child_pid = fork();
            if(child_pid == 0) {
                handle_child_process(argv[i]);
                exit(0);
            } else if(child_pid > 0) {
                // Wait for the child process to finish and get its exit status
                int child_status;
                waitpid(child_pid, &child_status, 0);
                if (WIFEXITED(child_status)) {
                    printf("The process with PID %d has ended with the exit code %d\n", child_pid, WEXITSTATUS(child_status));
                }
            } else {
                printf("Failed to fork second child process.\n");
                exit(1);
            }

            exit(0);
        } else {
            // In parent process, wait for child process to finish
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                printf("The process with PID %d has ended with the exit code %d\n", pid, WEXITSTATUS(status));
            }
        }
    }

    return 0;
}

void handle_regular_file(char *path) {
    while (1) {
        char option[100];

        printf("\nFile: %s\n", path);
        printf("Options:\n");
        printf("-n: Display name\n");
        printf("-d: Display size\n");
        printf("-h: Display hard link count\n");
        printf("-m: Display time of last modification\n");
        printf("-a: Display access rights\n");
        printf("-l: Create symbolic link (will wait for user input for the name of the link)\n");
        printf("Enter your option: ");

        scanf("%s", option);

        switch (option[1]) {
            case 'n':
                printf("File name: %s\n", path);
                break;

            case 'd':
                print_file_size(path);
                break;

            case 'h':
                print_hard_link_count(path);
                break;

            case 'm':
                print_last_modification_time(path);
                break;

            case 'a': {
                struct stat file_stat;
                if(stat(path, &file_stat) < 0) {
                    perror("stat");
                    exit(1);
                }

                print_access_rights(file_stat);
                break;
            }   

            case 'l': {
                char link_path[100];
                printf("Enter the name for the symbolic link: ");
                scanf("%s", link_path);
                create_symbolic_link(path, link_path);
                break;
            }

            default:{
                printf("Invalid option. Please try again.\n");
                continue;
            }
        }
        break;
    }
}


void handle_symlink(char *path) {
    char option[10];
    char target[1024];
    ssize_t len;
    struct stat symlink_stat, target_stat;

    if(lstat(path, &symlink_stat) < 0) {
        perror("lstat");
        exit(1);
    }

    if((len = readlink(path, target, sizeof(target)-1)) != -1) {
        target[len] = '\0';
    } else {
        perror("readlink");
        exit(1);
    }

    if(stat(target, &target_stat) < 0) {
        perror("stat");
        exit(1);
    }

    while(1) {
        printf("Select option for symbolic link %s:\n", path);
        printf("Name (-n)\n");
        printf("Delete symbolic link (-l)\n");
        printf("Size of symbolic link (-d)\n");
        printf("Size of target file (-t)\n");
        printf("Access rights (-a)\n");

        fgets(option, sizeof(option), stdin);
        option[strcspn(option, "\n")] = '\0';  // Strip newline character

        if(strcmp(option, "-n") == 0) {
            printf("Name: %s\n", path);
        } else if(strcmp(option, "-l") == 0) {
            printf("The option -l for deleting a symbolic link is not implemented in this function.\n");
            continue;
        } else if(strcmp(option, "-d") == 0) {
            printf("Size of symbolic link: %ld bytes\n", (long) symlink_stat.st_size);
        } else if(strcmp(option, "-t") == 0) {
            printf("Size of target file: %ld bytes\n", (long) target_stat.st_size);
        } else if(strcmp(option, "-a") == 0) {
            printf("The option -a for displaying access rights is not implemented in this function.\n");
            continue;
        } else {
            printf("Invalid option. Please try again.\n");
            continue;
        }

        break;
    }
}

void handle_directory(char *path) {
    char option[10];
    struct stat dir_stat;
    DIR *dirp;
    struct dirent *entry;
    int c_file_count = 0;

    if(stat(path, &dir_stat) < 0) {
        perror("stat");
        exit(1);
    }

    dirp = opendir(path);
    if(dirp == NULL) {
        perror("opendir");
        exit(1);
    }

    while((entry = readdir(dirp)) != NULL) {
        if(strstr(entry->d_name, ".c") != NULL) {
            c_file_count++;
        }
    }

    closedir(dirp);

    while(1) {
        printf("Select option for directory %s:\n", path);
        printf("Name (-n)\n");
        printf("Size (-d)\n");
        printf("Access rights (-a)\n");
        printf("Total number of .c files (-c)\n");

        fgets(option, sizeof(option), stdin);
        option[strcspn(option, "\n")] = '\0';  // Strip newline character

        if(strcmp(option, "-n") == 0) {
            printf("Name: %s\n", path);
        } else if(strcmp(option, "-d") == 0) {
            printf("Size of directory: %ld bytes\n", (long) dir_stat.st_size);
        } else if(strcmp(option, "-a") == 0) {
            printf("Access rights:\n");
            printf("User: read - %s, write - %s, execute - %s\n",
                   (dir_stat.st_mode & S_IRUSR) ? "yes" : "no",
                   (dir_stat.st_mode & S_IWUSR) ? "yes" : "no",
                   (dir_stat.st_mode & S_IXUSR) ? "yes" : "no");
            printf("Group: read - %s, write - %s, execute - %s\n",
                   (dir_stat.st_mode & S_IRGRP) ? "yes" : "no",
                   (dir_stat.st_mode & S_IWGRP) ? "yes" : "no",
                   (dir_stat.st_mode & S_IXGRP) ? "yes" : "no");
            printf("Others: read - %s, write - %s, execute - %s\n",
                   (dir_stat.st_mode & S_IROTH) ? "yes" : "no",
                   (dir_stat.st_mode & S_IWOTH) ? "yes" : "no",
                   (dir_stat.st_mode & S_IXOTH) ? "yes" : "no");
        } else if(strcmp(option, "-c") == 0) {
            printf("Total number of .c files: %d\n", c_file_count);
        } else {
            printf("Invalid option. Please try again.\n");
            continue;
        }

        break;
    }
}




void handle_child_process(char *path) {
    struct stat path_stat;

    if(stat(path, &path_stat) < 0) {
        perror("stat");
        exit(1);
    }

    if(S_ISREG(path_stat.st_mode)) {
        if(strstr(path, ".c") != NULL) {
            // Call script to compile .c file and print number of errors and warnings
            execlp("bash", "bash", "script.sh", path, NULL);
            perror("execlp");
            exit(1);
        } else {
            // Print number of lines in file
            execlp("wc", "wc", "-l", path, NULL);
            perror("execlp");
            exit(1);
        }
    } else if(S_ISDIR(path_stat.st_mode)) {
        // Create .txt file
        char file_name[1024];
        snprintf(file_name, sizeof(file_name), "%s_file.txt", path);
        execlp("touch", "touch", file_name, NULL);
        perror("execlp");
        exit(1);
    }
}


void print_access_rights(struct stat path_stat) {
    printf("Access rights:\n");
    printf("User: read - %s, write - %s, execute - %s\n",
           (path_stat.st_mode & S_IRUSR) ? "yes" : "no",
           (path_stat.st_mode & S_IWUSR) ? "yes" : "no",
           (path_stat.st_mode & S_IXUSR) ? "yes" : "no");
    printf("Group: read - %s, write - %s, execute - %s\n",
           (path_stat.st_mode & S_IRGRP) ? "yes" : "no",
           (path_stat.st_mode & S_IWGRP) ? "yes" : "no",
           (path_stat.st_mode & S_IXGRP) ? "yes" : "no");
    printf("Others: read - %s, write - %s, execute - %s\n",
           (path_stat.st_mode & S_IROTH) ? "yes" : "no",
           (path_stat.st_mode & S_IWOTH) ? "yes" : "no",
           (path_stat.st_mode & S_IXOTH) ? "yes" : "no");
}


void execute_script(char *script_path, char *c_file_path) {
    int fd[2];
    pid_t pid;
    char buf[1024];

    // Create a pipe
    if(pipe(fd) < 0) {
        perror("pipe");
        exit(1);
    }

    pid = fork();
    if(pid < 0) {
        perror("fork");
        exit(1);
    } else if(pid == 0) {  // Child process
        // Close the read end of the pipe
        close(fd[0]);

        // Redirect stdout to the write end of the pipe
        dup2(fd[1], STDOUT_FILENO);

        // Execute the script
        execlp("bash", "bash", script_path, c_file_path, NULL);
        perror("execlp");
        exit(1);
    } else {  // Parent process
        // Close the write end of the pipe
        close(fd[1]);

        // Read the script output from the read end of the pipe
        while(read(fd[0], buf, sizeof(buf)) > 0) {
            printf("%s\n", buf);
        }

        // Wait for the child process to finish
        wait(NULL);
    }
}


int count_lines(char *file_path) {
    FILE *file = fopen(file_path, "r");
    if(file == NULL) {
        perror("fopen");
        return -1;
    }

    int count = 0;
    char c;
    while((c = fgetc(file)) != EOF) {
        if(c == '\n') {
            count++;
        }
    }

    fclose(file);
    return count;
}

void create_symbolic_link(char *path, char *link_path) {
    if(symlink(path, link_path) < 0) {
        perror("symlink");
        exit(1);
    }

    printf("Symbolic link created successfully.\n");
}

void print_last_modification_time(char *path) {
    struct stat file_stat;
    if(stat(path, &file_stat) < 0) {
        perror("stat");
        exit(1);
    }

    char time_str[100];
    strftime(time_str, sizeof(time_str), "%d-%m-%Y %H:%M:%S", localtime(&file_stat.st_mtime));

    printf("Last modification time: %s\n", time_str);
}

void print_hard_link_count(char *path) {
    struct stat file_stat;
    if(stat(path, &file_stat) < 0) {
        perror("stat");
        exit(1);
    }

    printf("Hard link count: %lu\n", (unsigned long) file_stat.st_nlink);
}

void print_file_size(char *path) {
    struct stat file_stat;
    if(stat(path, &file_stat) < 0) {
        perror("stat");
        exit(1);
    }

    printf("File size: %ld bytes\n", (long) file_stat.st_size);
}
