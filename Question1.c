#include <unistd.h>
#include <string.h>



#define WELCOME_MESSAGE "Welcome to ENSEA Tiny Shell.\nType 'exit' to quit.\n"
#define PROMPT "enseash % "
#define READ_BUFFER_SIZE 128

int main(void) {
    char input_buffer[READ_BUFFER_SIZE];
    ssize_t read_size;
    write(STDOUT_FILENO, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE));
    while (1) {
        /* Display prompt */
        write(STDOUT_FILENO, PROMPT, strlen(PROMPT));

        /* Read user input */
        read_size = read(STDIN_FILENO, input_buffer, READ_BUFFER_SIZE - 1);

        if (read_size <= 0) {
            /* End of file or read error */
            break;
        }

        input_buffer[read_size] = '\0';

        /* Remove newline */
        if (read_size > 0 && input_buffer[read_size - 1] == '\n') {
            input_buffer[read_size - 1] = '\0';
        }
        /* Exit command */
        if (strncmp(input_buffer, "exit", 4) == 0) {
            break;
        }
    }
    return 0;
}
