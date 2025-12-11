#include <unistd.h>
#include <string.h>
#include <sys/wait.h>



#define WELCOME_MESSAGE "Welcome to ENSEA Tiny Shell.\nType 'exit' to quit.\n"
#define PROMPT "enseash % "
#define READ_BUFFER_SIZE 128
#define EXIT_CMD "exit"
#define EXIT_CMD_LENGTH 4

int main(void) {
    char input_buffer[READ_BUFFER_SIZE];
    ssize_t read_size;
    pid_t child_pid;
    int child_status;

    write(STDOUT_FILENO, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE));

    while (1) {
        write(STDOUT_FILENO, PROMPT, strlen(PROMPT));

        read_size = read(STDIN_FILENO, input_buffer, READ_BUFFER_SIZE - 1);

        if (read_size <= 0) {
            /* End of file or read error */
            break;
        }

        input_buffer[read_size] = '\0';
        if (read_size > 0 && input_buffer[read_size - 1] == '\n') {
            input_buffer[read_size - 1] = '\0';
        }

        /* Exit command */
        if (strncmp(input_buffer, EXIT_CMD, EXIT_CMD_LENGTH) == 0) {
            break;
        }
        child_pid = fork();
        if (child_pid == 0) {
            /* Child: execute the command */
            char *command = input_buffer;
            char *argv[] = { command, NULL };

            execvp(command, argv);

            _exit(1);
        } else if (child_pid > 0) {
            /* Parent: wait for child */
            wait(&child_status);
        } else {
            const char *error_msg = "Error: fork failed.\n";
            write(STDERR_FILENO, error_msg, strlen(error_msg));
        }
    }
    return 0;
}
