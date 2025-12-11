#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>

#define MAX_CMD_SIZE 128
#define MAX_PROMPT_SIZE 128
#define WELCOME_MESSAGE "Welcome to ENSEA Tiny Shell.\nType 'exit' to quit.\n"
#define PROMPT_SIMPLE "enseash % "
#define EXIT_MESSAGE "Bye bye...\n"

int main(void) {
    char cmd_buffer[MAX_CMD_SIZE];
    char prompt_buffer[MAX_PROMPT_SIZE];

    ssize_t read_bytes;
    int status = 0;
    struct timespec start_time;
    struct timespec end_time;

    write(STDOUT_FILENO, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE));
    write(STDOUT_FILENO, PROMPT_SIMPLE, strlen(PROMPT_SIMPLE));
    while (1) {
        read_bytes = read(STDIN_FILENO, cmd_buffer, MAX_CMD_SIZE);

        /* Ctrl+D (EOF) */
        if (read_bytes == 0) {
            write(STDOUT_FILENO, EXIT_MESSAGE, strlen(EXIT_MESSAGE));
            return 0;
        }
        
        if (cmd_buffer[read_bytes - 1] == '\n') {
            cmd_buffer[read_bytes - 1] = '\0';
        }

        if (strncmp(cmd_buffer, "exit", 4) == 0) {
            write(STDOUT_FILENO, EXIT_MESSAGE, strlen(EXIT_MESSAGE));
            return 0;
        }

        clock_gettime(CLOCK_MONOTONIC, &start_time);

        pid_t pid = fork();

        if (pid == 0) {

            char *args[] = { cmd_buffer, NULL };
            execvp(cmd_buffer, args);
            const char *errmsg = "Command not found\n";
            write(STDERR_FILENO, errmsg, strlen(errmsg));
            exit(127);
        }

        wait(&status);
        clock_gettime(CLOCK_MONOTONIC, &end_time);

        long elapsed_ms =
            (end_time.tv_sec - start_time.tv_sec) * 1000 +
            (end_time.tv_nsec - start_time.tv_nsec) / 1000000;

        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            int len = snprintf(prompt_buffer, MAX_PROMPT_SIZE,
                               "enseash [exit:%d|%ldms] %% ", code, elapsed_ms);
            write(STDOUT_FILENO, prompt_buffer, len);
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            int len = snprintf(prompt_buffer, MAX_PROMPT_SIZE,
                               "enseash [sign:%d|%ldms] %% ", sig, elapsed_ms);
            write(STDOUT_FILENO, prompt_buffer, len);
        } else {
            write(STDOUT_FILENO, PROMPT_SIMPLE, strlen(PROMPT_SIMPLE));
        }
    }
    return 0;
}
