#include <unistd.h>
#include <string.h>
#include <sys/wait.h>



#define WELCOME_MESSAGE "Welcome to ENSEA Tiny Shell.\nType 'exit' to quit.\n"
#define PROMPT "enseash % "
#define GOODBYE_MESSAGE "Bye bye...\n"
#define READ_BUFFER_SIZE 128
#define EXIT_CMD "exit"
#define EXIT_CMD_LENGTH 4

int main(void) {
    char input_buffer[READ_BUFFER_SIZE];
    ssize_t read_size;
    pid_t child_pid;
    int child_status;

    /* Display welcome message */
    write(STDOUT_FILENO, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE));

    while (1) {
        /* Display prompt */
        write(STDOUT_FILENO, PROMPT, strlen(PROMPT));

        read_size = read(STDIN_FILENO, input_buffer, READ_BUFFER_SIZE - 1);

        if (read_size == 0) {
            /* CTRL+D detected */
            write(STDOUT_FILENO, GOODBYE_MESSAGE, strlen(GOODBYE_MESSAGE));
            break;
        }

        if (read_size < 0) {
            /* Read error */
            write(STDOUT_FILENO, GOODBYE_MESSAGE, strlen(GOODBYE_MESSAGE));
            break;
        }

        input_buffer[read_size] = '\0';

        /* Remove newline */
        if (read_size > 0 && input_buffer[read_size - 1] == '\n') {
            input_buffer[read_size - 1] = '\0';
        }

        /* "exit" command */
        if (strncmp(input_buffer, EXIT_CMD, EXIT_CMD_LENGTH) == 0) {
            write(STDOUT_FILENO, GOODBYE_MESSAGE, strlen(GOODBYE_MESSAGE));
            break;
        }

        /* Fork and execute command */
        child_pid = fork();

        if (child_pid == 0) {
            char *command = input_buffer;
            char *argv[] = { command, NULL };
            execvp(command, argv);
            _exit(1);  /* exec failed */
        } else if (child_pid > 0) {
            wait(&child_status);
        } else {
            const char *error_msg = "Error: fork failed.\n";
            write(STDERR_FILENO, error_msg, strlen(error_msg));
        }
    }
    return 0;
}
