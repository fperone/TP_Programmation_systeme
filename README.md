# Students: Felipe Perone and Vinicius Baião Pires.
# Overview
This project consists in the implementation of a Unix shell, developed in C using low-level system calls. The shell basically allows the user to execute commands, display execution information and other functionalities. The implementation was done step by step, adding one feature at a time, creating a new file for everysingle feature added, meaning that the Question7.c file contains the code with all the implemented features.
All constraints given in the project were respected
- No use of printf
- No use of system
- Use of strn* string functions

# Question1.c
The first step was to display a welcome message and also a simple prompt when the shell star.
This was done using:
write() to display messages
A simple infinite loop to keep the shell running

# Question2.c 
The shell was then extended to:
- Read a command from standard input
- Execute the command
- Return to the prompt
- This created a basic Read Execute Print Loop.
In order to that, it was needed:
- read() to get user input
- fork() to create a child process
- execvp() to execute the command
- wait() in the parent process
At this step, only simple commands without arguments were supported.

# Question3.c

The shell was improved to exit cleanly in two cases:
When the user types exit (also including when the user presses Ctrl + D)
In both cases, the shell prints the exit message: "Bye bye..."
This was detected by comparing the input string with "exit" and checking if read() returns 0 (End of the File)

# Question4.c

After executing a command, the shell displays the exit status of the previous command directly in the prompt.
For example:
enseash [exit:0] %
enseash [sign:9] %
Implementation details:
- wait() status analysis
- WIFEXITED and WEXITSTATUS for normal exit
- WIFSIGNALED and WTERMSIG for signal termination
- Prompt dynamically rebuilt using snprintf

# Quetion5.c

The shell was then extended to measure the execution time of each command.
Example: 
enseash [exit:0|10ms] %
enseash [sign:9|5ms] %
Implementation details:
- clock_gettime(CLOCK_MONOTONIC, ...)
- Time measured before fork() and after wait()
- Time converted to milliseconds
- Execution time stored and shown in the next prompt

# Question6.c

Support for commands with arguments was added.
For example:
enseash % hostname -i
enseash % fortune -s osfortune
Important implementation choices:
- Use of strtok() to tokenize the command line
- Arguments are separated using spaces as delimiters
- Construction of the argv[] array dynamically from parsed tokens
- Proper NULL termination of argv[] before calling execvp()
- Full compatibility with previously implemented features (fork, exec, timing, prompt)

This implementation allows the shell to execute any external command with an arbitrary number of arguments.

# Question7.c

Finally, the shell was extended to support basic input and output redirections.
Examples: 
enseash % ls > filelist.txt
enseash % wc -l < filelist.txt
Implementation details:
- Parsing of the command line using strtok()
- Detection of redirection operators < and > during token parsing
- Identification of the input and output filenames following the redirection symbols
- Removal of redirection tokens from the argv[] array
- Use of open() to open files:
- O_RDONLY for input redirection
- O_WRONLY | O_CREAT | O_TRUNC for output redirection
- Use of dup2() to redirect:
- STDIN_FILENO for <
- STDOUT_FILENO for >
- Redirections are applied only in the child process, before calling execvp()
  
Only simple redirections are supported; pipes and advanced redirection combinations are intentionally not handled.
