#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <time.h>

#define MAX_CMD_SIZE 128
#define MAX_PROMPT_SIZE 128
#define WELCOME_MESSAGE "Welcome to ENSEA Tiny Shell.\nType 'exit' to quit.\n"
#define PROMPT_SIMPLE "enseash % "
#define EXIT_MESSAGE "Bye bye...\n"

#define READ_BUFFER_SIZE 256
#define MAX_PROMPT_SIZE 128
#define MAX_ARGS (READ_BUFFER_SIZE / 2)

#define WELCOME_MESSAGE "Welcome to ENSEA Tiny Shell.\nType 'exit' to quit.\n"
#define GOODBYE_MESSAGE "Bye bye...\n"

#define PROMPT_DEFAULT "enseash % "
#define PROMPT_EXIT_PREFIX "enseash [exit:"
#define PROMPT_SIGN_PREFIX "enseash [sign:"
#define PROMPT_SUFFIX "] % "

#define EXIT_CMD "exit"
#define EXIT_CMD_LENGTH 4

#define CMDNOTFOUND_MSG "Command not found.\n"
#define CMDNOTFOUND_MSG_LENGTH 19

int main(void) {
    char input_buffer[MAX_CMD_SIZE];
    char prompt_buffer[MAX_PROMPT_SIZE];

    ssize_t read_size;
    pid_t child_pid;
    int child_status = 0;

    int last_exit_code = 0;
    int last_signal = 0;
    long last_time_ms = 0;
    int first_prompt = 1;

    struct timespec time_start;
    struct timespec time_end;

    write(STDOUT_FILENO, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE)); // Welcome message
    write(STDOUT_FILENO, PROMPT_SIMPLE, strlen(PROMPT_SIMPLE));

    while (1) {
        if (first_prompt) { // First condition made to display the prefix of the first prompt
            strncpy(prompt_buffer, PROMPT_DEFAULT, MAX_PROMPT_SIZE - 1); // copy of the content in PROMPT_DEFAULT to prompt_buffer up until MAX_PROMPT_SIZE bytes
            prompt_buffer[MAX_PROMPT_SIZE - 1] = '\0'; // ensure null-termination
            first_prompt = 0;
        } else if (last_signal != 0) { // Second condition made to display the prefix of the prompt when the last command was terminated by a signal
            snprintf(prompt_buffer, MAX_PROMPT_SIZE, "%s%d|%ldms%s", PROMPT_SIGN_PREFIX, last_signal, last_time_ms, PROMPT_SUFFIX);
        } else { // Third condition made to display the prefix of the prompt when the last command exited normally
            snprintf(prompt_buffer, MAX_PROMPT_SIZE, "%s%d|%ldms%s", PROMPT_EXIT_PREFIX, last_exit_code, last_time_ms, PROMPT_SUFFIX);
        }

        write(STDOUT_FILENO, prompt_buffer, strlen(prompt_buffer)); // display the prompt using write system call
        read_size = read(STDIN_FILENO, input_buffer, MAX_CMD_SIZE);

        if (read_size == 0) {
            write(STDOUT_FILENO, EXIT_MESSAGE, strlen(EXIT_MESSAGE));
            return 0;
        }
        if (input_buffer[read_size - 1] == '\n') { // handle newline character at the end of input in case user pressed Enter
            input_buffer[read_size - 1] = '\0';
        }
        if (strncmp(input_buffer, EXIT_CMD, EXIT_CMD_LENGTH) == 0) { // check for exit command by comparing input with the "exit" string
            write(STDOUT_FILENO, GOODBYE_MESSAGE, strlen(GOODBYE_MESSAGE));
            break;
        }
        clock_gettime(CLOCK_MONOTONIC, &time_start); // start timing the command execution
        child_pid = fork(); // create a new child process to execute the command

        if (child_pid == 0) {
            char *args[] = { input_buffer, NULL };
            execvp(input_buffer, args);

            const char *errmsg = "Command not found\n";
            write(STDERR_FILENO, errmsg, strlen(errmsg));
            exit(127);
        }
        wait(&child_status);
        clock_gettime(CLOCK_MONOTONIC, &time_end);

        last_time_ms = (time_end.tv_sec - time_start.tv_sec) * 1000 + (time_end.tv_nsec - time_start.tv_nsec) / 1000000;

        if (WIFEXITED(child_status)) { // check if the child process exited normally
            last_exit_code = WEXITSTATUS(child_status);
            last_signal = 0;
        } else if (WIFSIGNALED(child_status)) { // check if the child process was terminated by a signal
            last_signal = WTERMSIG(child_status);
            last_exit_code = 0;
        }
    }
    return 0;
}
