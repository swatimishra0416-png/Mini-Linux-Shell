#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#define MAX_COMMANDS 10
#define MAX_COMMAND_LENGTH 100

void execute_command(char* commands[MAX_COMMANDS], int num_commands) {
    int pipe_fd[2];
    int prev_read = STDIN_FILENO;

    for (int i = 0; i < num_commands; i++) {
        if (i < num_commands - 1) {
            pipe(pipe_fd);
        }

        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process

            // Redirect input
            if (i > 0) {
                dup2(prev_read, STDIN_FILENO);
                close(prev_read);
            }

            // Redirect output
            if (i < num_commands - 1) {
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[0]);
                close(pipe_fd[1]);
            }

            // Tokenize the command into executable and arguments
            char* executable = strtok(commands[i], " ");
            char* arguments[MAX_COMMANDS];
            arguments[0] = executable;

            int arg_index = 1;
            char* arg = strtok(NULL, " ");
            while (arg != NULL) {
                arguments[arg_index++] = arg;
                arg = strtok(NULL, " ");
            }
            arguments[arg_index] = NULL;

            // Execute the command
            if (execvp(executable, arguments) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else {
            // Parent process
            wait(NULL);

            // Close the write end of the pipe if it's not the last command
            if (i < num_commands - 1) {
                close(pipe_fd[1]);
                prev_read = pipe_fd[0];
            }
        }
    }
}

int main() {
    while (1) {
        printf("$ ");

        char input[MAX_COMMAND_LENGTH];
        fgets(input, sizeof(input), stdin);

        // Remove newline character from input
        size_t input_length = strlen(input);
        if (input_length > 0 && input[input_length - 1] == '\n') {
            input[input_length - 1] = '\0';
        }

        // Tokenize the input into commands
        char* commands[MAX_COMMANDS];
        char* token = strtok(input, "|");
        int num_commands = 0;

        while (token != NULL) {
            commands[num_commands++] = token;
            token = strtok(NULL, "|");
        }

        // Execute the commands
        execute_command(commands, num_commands);
    }

    return 0;
}
