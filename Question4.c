#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdio.h>


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
  
    write(STDOUT_FILENO, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE)); // Display welcome message
    while (1) {
        if (first_prompt) {

            strncpy(prompt_buffer, PROMPT_DEFAULT, MAX_PROMPT_SIZE - 1); // Set default prompt
            prompt_buffer[MAX_PROMPT_SIZE - 1] = '\0'; // Ensure null-termination
            first_prompt = 0;
        } else if (last_signal != 0) { // If last command ended with a signal
            int length = snprintf(prompt_buffer, MAX_PROMPT_SIZE, 
                                  "%s%d%s", 
                                  PROMPT_SIGN_PREFIX, 
                                  last_signal, 
                                  PROMPT_SUFFIX); 
            (void)length; 
        } else {
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
            write(STDOUT_FILENO, GOODBYE_MESSAGE, strlen(GOODBYE_MESSAGE));
            break;
        }

        if (read_size < 0) {
            write(STDOUT_FILENO, GOODBYE_MESSAGE, strlen(GOODBYE_MESSAGE));
            break;
        }

        input_buffer[read_size] = '\0'; // Null-terminate the input string

        if (input_buffer[read_size - 1] == '\n') {
            input_buffer[read_size - 1] = '\0';
        }

        if (strncmp(input_buffer, EXIT_CMD, EXIT_CMD_LENGTH) == 0) {
            write(STDOUT_FILENO, GOODBYE_MESSAGE, strlen(GOODBYE_MESSAGE));
            break; // Exit the shell
        }

        child_pid = fork(); 
        if (child_pid == 0) { 
            char *argv[READ_BUFFER_SIZE / 2 + 2];
            int arg_count = 0;
            int index = 0;
            int length = strnlen(input_buffer, READ_BUFFER_SIZE);
        
            while (index < length) {
        
                while (index < length && input_buffer[index] == ' ') { // Skip spaces
                    index++;
                }
        
                if (index >= length) {
                    break;
                }

                argv[arg_count] = &input_buffer[index]; // Set argument pointer
                arg_count++;
        
                while (index < length && input_buffer[index] != ' ') { // Move to next space
                    index++;
                }
        
                if (index < length) { // Null-terminate the argument
                    input_buffer[index] = '\0';
                    index++;
                }
            }
        
            argv[arg_count] = NULL; // Null-terminate the argument list
        
            execvp(argv[0], argv);
        
            write(STDERR_FILENO, "Command not found.\n", 19);
            _exit(1);
        }

        wait(&child_status); // Wait for child process to finish
        if (WIFEXITED(child_status)) {
            last_exit_code = WEXITSTATUS(child_status);
            last_signal = 0;
        } else if (WIFSIGNALED(child_status)) { 
            last_signal = WTERMSIG(child_status); // if terminated by a signal get terminating signal
            last_exit_code = 0;
        }
    }
    return 0;
}
