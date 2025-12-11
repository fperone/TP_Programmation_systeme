#include <unistd.h>
#include <string.h>
#include <sys/wait.h>


#define WELCOME_MESSAGE "Welcome to ENSEA Tiny Shell.\nType 'exit' to quit.\n"
#define PROMPT_DEFAULT "enseash % "
#define PROMPT_EXIT_PREFIX "enseash [exit:"
#define PROMPT_SIGN_PREFIX "enseash [sign:"
#define PROMPT_SUFFIX "] % "
#define GOODBYE_MESSAGE "Bye bye...\n"
#define READ_BUFFER_SIZE 128
#define EXIT_CMD "exit"
#define EXIT_CMD_LENGTH 4
#define MAX_PROMPT_SIZE 64

int main(void) {
    char input_buffer[READ_BUFFER_SIZE];
    char prompt_buffer[MAX_PROMPT_SIZE];
    ssize_t read_size;

    pid_t child_pid;
    int child_status;
    int last_exit_code = 0;
    int last_signal = 0;
    int first_prompt = 1;
  
    write(STDOUT_FILENO, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE));
    while (1) {
        if (first_prompt) {

            strncpy(prompt_buffer, PROMPT_DEFAULT, MAX_PROMPT_SIZE - 1);
            prompt_buffer[MAX_PROMPT_SIZE - 1] = '\0';
            first_prompt = 0;
        } else if (last_signal != 0) {
            /* Signal-based termination */
            int length = snprintf(prompt_buffer, MAX_PROMPT_SIZE, 
                                  "%s%d%s", 
                                  PROMPT_SIGN_PREFIX, 
                                  last_signal, 
                                  PROMPT_SUFFIX);
            (void)length; /* suppress unused variable warning */
        } else {
            /* Normal exit */
            int length = snprintf(prompt_buffer, MAX_PROMPT_SIZE, 
                                  "%s%d%s", 
                                  PROMPT_EXIT_PREFIX, 
                                  last_exit_code, 
                                  PROMPT_SUFFIX);
            (void)length;
        }

        write(STDOUT_FILENO, prompt_buffer, strlen(prompt_buffer));

        read_size = read(STDIN_FILENO, input_buffer, READ_BUFFER_SIZE - 1);

        if (read_size == 0) {
            /* CTRL+D */
            write(STDOUT_FILENO, GOODBYE_MESSAGE, strlen(GOODBYE_MESSAGE));
            break;
        }

        if (read_size < 0) {
            /* Read error */
            write(STDOUT_FILENO, GOODBYE_MESSAGE, strlen(GOODBYE_MESSAGE));
            break;
        }

        input_buffer[read_size] = '\0';

        if (input_buffer[read_size - 1] == '\n') {
            input_buffer[read_size - 1] = '\0';
        }

        if (strncmp(input_buffer, EXIT_CMD, EXIT_CMD_LENGTH) == 0) {
            write(STDOUT_FILENO, GOODBYE_MESSAGE, strlen(GOODBYE_MESSAGE));
            break;
        }

        child_pid = fork();

        if (child_pid == 0) {
            char *command = input_buffer;
            char *argv[] = { command, NULL };
            execvp(command, argv);
            _exit(1); /* If execvp fails */
        }
        wait(&child_status);
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
