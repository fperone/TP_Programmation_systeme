#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <stdio.h>

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

int main(void) {
    char input_buffer[READ_BUFFER_SIZE];
    char prompt_buffer[MAX_PROMPT_SIZE];

    ssize_t read_size;
    pid_t child_pid;
    int child_status;

    int last_exit_code = 0;
    int last_signal = 0;
    long last_time_ms = 0;
    int first_prompt = 1;

    struct timespec time_start;
    struct timespec time_end;

    /* Welcome message */
    write(STDOUT_FILENO, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE));

    while (1) {

        /* ----- Prompt ----- */
        if (first_prompt) {
            strncpy(prompt_buffer, PROMPT_DEFAULT, MAX_PROMPT_SIZE - 1);
            prompt_buffer[MAX_PROMPT_SIZE - 1] = '\0';
            first_prompt = 0;
        } else if (last_signal != 0) {
            snprintf(prompt_buffer, MAX_PROMPT_SIZE,
                     "%s%d|%ldms%s",
                     PROMPT_SIGN_PREFIX,
                     last_signal,
                     last_time_ms,
                     PROMPT_SUFFIX);
        } else {
            snprintf(prompt_buffer, MAX_PROMPT_SIZE,
                     "%s%d|%ldms%s",
                     PROMPT_EXIT_PREFIX,
                     last_exit_code,
                     last_time_ms,
                     PROMPT_SUFFIX);
        }

        write(STDOUT_FILENO, prompt_buffer, strlen(prompt_buffer));

        /* ----- Read input ----- */
        read_size = read(STDIN_FILENO, input_buffer, READ_BUFFER_SIZE - 1);

        if (read_size == 0) {
            write(STDOUT_FILENO, GOODBYE_MESSAGE, strlen(GOODBYE_MESSAGE));
            break;
        }

        if (read_size < 0) {
            write(STDOUT_FILENO, GOODBYE_MESSAGE, strlen(GOODBYE_MESSAGE));
            break;
        }

        input_buffer[read_size] = '\0';

        if (input_buffer[read_size - 1] == '\n') {
            input_buffer[read_size - 1] = '\0';
        }

        /* Exit command */
        if (strncmp(input_buffer, EXIT_CMD, EXIT_CMD_LENGTH) == 0) {
            write(STDOUT_FILENO, GOODBYE_MESSAGE, strlen(GOODBYE_MESSAGE));
            break;
        }

        /* ----- Fork ----- */
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        child_pid = fork();

        if (child_pid == 0) {
            /* ----- CHILD: parse arguments using strtok ----- */
            char *argv[MAX_ARGS];
            int argc = 0;

            char *token = strtok(input_buffer, " ");
            while (token != NULL && argc < MAX_ARGS - 1) {
                argv[argc] = token;
                argc++;
                token = strtok(NULL, " ");
            }

            argv[argc] = NULL;

            execvp(argv[0], argv);

            write(STDERR_FILENO, "Command not found.\n", 19);
            _exit(1);
        }

        /* ----- Parent ----- */
        wait(&child_status);
        clock_gettime(CLOCK_MONOTONIC, &time_end);

        last_time_ms =
            (time_end.tv_sec - time_start.tv_sec) * 1000 +
            (time_end.tv_nsec - time_start.tv_nsec) / 1000000;

        if (WIFEXITED(child_status)) {
            last_exit_code = WEXITSTATUS(child_status);
            last_signal = 0;
        } else if (WIFSIGNALED(child_status)) {
            last_signal = WTERMSIG(child_status);
            last_exit_code = 0;
        }
    }

    return 0;
}
