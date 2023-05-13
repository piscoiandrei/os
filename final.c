#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

// Function to print the size of the string
void print_size(const char *str) {
    printf("Size of string: %lu\n", strlen(str));
}

// Function to reverse the string
void reverse_string(char *str) {
    int length, start, end;
    char temp;

    length = strlen(str);
    start = 0;
    end = length - 1;

    for (start = 0; start < end; start++, end--) {
        temp = str[start];
        str[start] = str[end];
        str[end] = temp;
    }

    printf("Reversed string: %s\n", str);
}

// Function to count the number of words in a string
int count_words(const char *str) {
    int count = 0;
    int in_word = 0;

    while (*str) {
        if (isspace(*str)) {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            ++count;
        }
        ++str;
    }

    return count;
}

int main() {
    int pipe_fd[2];
    pid_t pid;
    const char *str = "This is a simple word count script.";

    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {                      // Child process
        close(pipe_fd[1]); // Close write end of the pipe

        char buffer[1024];
        read(pipe_fd[0], buffer, sizeof(buffer));
        close(pipe_fd[0]); // Close read end of the pipe

        int word_count = count_words(buffer);
        printf("Word count: %d\n", word_count);

        reverse_string(buffer);
    } else {                      // Parent process
        close(pipe_fd[0]); // Close read end of the pipe

        print_size(str);
        write(pipe_fd[1], str, strlen(str) + 1);
        close(pipe_fd[1]); // Close write end of the pipe

        wait(NULL); // Wait for child process to complete
    }

    return 0;
}
