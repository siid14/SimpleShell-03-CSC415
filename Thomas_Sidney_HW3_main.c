#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> // include for pid_t
#include <sys/wait.h>  // include for waitpid
#include <unistd.h>    // include for fork and execvp
#include <errno.h>     // include errno to access the error code
#include <stdbool.h>   // Include this header for bool type
#include <ctype.h>

// ! don't forget to use strtok_r instead of strtok to not lose points

bool isWhiteSpaceInput(const char *input)
{
    // iterate through each character in the input string
    for (int i = 0; input[i] != '\0'; i++)
    {
        // check if the character is not a whitespace character
        if (!isspace(input[i]))
        {
            return false; // if any non-whitespace character is found, return false
        }
    }
    return true; // if all characters are whitespace, return true
}

// maximum number of commands in a pipeline
#define MAX_COMMANDS 10

int main(int argumentCount, char *argumentValues[])
{
    // * READ USER INPUT
    char input[187];                // store user input command of 187 characters
    char *saveptr;                  // ave pointer for strtok_r
    int pipes[MAX_COMMANDS - 1][2]; // array to store pipe file descriptors

    while (1) // infinite loop
    {
        printf("YourShell> "); // indicate shell ready to accept user input
                               // ? unacessary - redundant
                               // fgets(input, sizeof(input), stdin); // read the user input + store in input

        // read user input + handle error
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            // check if fgets encountered EOF
            if (feof(stdin))
            {
                // gracefully exit on EOF without reporting an error
                printf("EOF error - Exiting the shell\n");
                return 0; // ? break to use instead
            }
            else
            {
                perror("fgets"); // report fgets errors
                exit(1);         // exit the shell on error
            }
        }

        // handle empty input - edge case
        if (strlen(input) == 1)
        {
            printf("Empty string entered, please enter a valid string\n");
            perror("Empty string error");
            continue;
        }

        // Check for whitespace input
        if (isWhiteSpaceInput(input))
        {
            printf("Blank line entered, please enter a valid command\n");
            continue;
        }

        printf("Input size (including newline): %zu\n", strlen(input));
        printf("Input size (excluding newline): %zu\n", strlen(input) - 1);

        // check for pipes in user input
        char *pipe_cmd = strchr(input, '|');

        if (pipe_cmd)
        {
            printf("PIPE DETECTED\n");
            // handle piping logic
            // parse and execute commands separated by pipes
            // create pipes, fork processes, and redirect IO

            // split input into commands based on '|'
            char *command = strtok_r(input, "|", &saveptr);
            int pipe_index = 0; // initialize the pipe index
            while (command != NULL)
            {
                // execute each command
                printf("Command: %s\n", command);

                // trim leading and trailing spaces from the command
                char *trimmed_command = strtok(command, " \t\r\n");
                if (trimmed_command)
                {
                    printf("Trimmed Command: %s\n", trimmed_command);
                }

                // create a pipe for the next pair of commands (if applicable)
                if (pipe_index < MAX_COMMANDS - 1)
                {
                    if (pipe(pipes[pipe_index]) == -1)
                    {
                        perror("pipe");
                        exit(1);
                    }
                    printf("Pipe %d created\n", pipe_index);
                }

                // fork a child process for the current command
                pid_t child_pid = fork();
                if (child_pid == -1)
                {
                    perror("fork");
                    exit(1);
                }

                if (child_pid == 0)
                {
                    // child process
                    if (pipe_index > 0)
                    {
                        // redirect standard input (stdin) to the read end of the previous pipe
                        dup2(pipes[pipe_index - 1][0], 0);
                        close(pipes[pipe_index - 1][0]); // close the read end of the previous pipe
                        printf("Child: Redirected stdin to read end of Pipe %d\n", pipe_index - 1);
                    }

                    if (pipe_index < MAX_COMMANDS - 1)
                    {
                        // redirect standard output (stdout) to the write end of the current pipe
                        dup2(pipes[pipe_index][1], 1);
                        close(pipes[pipe_index][1]); // close the write end of the current pipe
                        printf("Child: Redirected stdout to write end of Pipe %d\n", pipe_index);
                    }

                    // close all other pipe ends in the child process
                    for (int i = 0; i < MAX_COMMANDS - 1; i++)
                    {
                        if (i != pipe_index - 1 && i != pipe_index)
                        {
                            close(pipes[i][0]);
                            close(pipes[i][1]);
                        }
                    }

                    // execute the current command
                    execlp(command, command, NULL);
                    perror("exec");
                    exit(1);
                }
                else
                {
                    // parent process
                    // close pipe ends in the parent process
                    if (pipe_index > 0)
                    {
                        close(pipes[pipe_index - 1][0]);
                        close(pipes[pipe_index - 1][1]);
                        printf("Closed pipe %d ends\n", pipe_index - 1);
                    }

                    pipe_index++;
                }

                // move to the next command
                command = strtok_r(NULL, "|", &saveptr);
            }

            // close unused pipe ends
            for (int i = 0; i < pipe_index; i++)
            {
                // close the write end of the previous pipe
                close(pipes[i][1]);
                printf("Write end of Pipe %d closed\n", i);

                // close the read end of the current pipe
                close(pipes[i][0]);
                printf("Read end of Pipe %d closed\n", i);
            }
        }
        else // handle non-piping logic
        {
            printf("NO PIPE DETECTED\n");
            // remove trailing newline character if present
            if (input[strlen(input) - 1] == '\n') // check if last char in input is a newline character
            {
                input[strlen(input) - 1] = '\0'; // replace '\n' with null terminator '\0'
            }
            // }

            // * PARSE USER INPUT
            char *command = strtok(input, " "); // tokenize first string of user input
            printf("Command: %s\n", command);

            // handle exit input to exit shell (edge case)
            if (strcmp(command, "exit") == 0 || strcmp(command, "EXIT") == 0)
            {
                printf("User type 'exit' - Exiting the shell\n");
                break; // exit the loop and terminate the shell
            }

            char *args[32];
            // * store remaining input string (arguments) into args
            int arg_count = 0;

            while ((args[arg_count] = strtok(NULL, " "))) // tokenize each remaining string of input
                                                          // until no more found
            {
                arg_count++;
            }
            // printf("Number of argument in args: %d\n", arg_count);

            // int argsSize = sizeof(args) / sizeof(args[0]);
            // printf("Number of elements in args: %d\n", argsSize);

            char **argsTest;
            int len = arg_count; // number of strings to store
            // printf("len: %d\n", len);
            argsTest = malloc(sizeof(char *) * len);

            // check if memory allocation was successful
            if (argsTest == NULL)
            {
                perror("Memory allocation failed");
                free(argsTest); // free the allocated memory
                return 1;
            }

            // copy args content into argsTest
            for (int i = 0; i < len; i++)
            {
                argsTest[i] = args[i];
                // printf("argsTest[%d]: %s\n", i, argsTest[i]);
            }

            // ? unaccessary
            // check if memory allocation for strings was successful
            for (int i = 0; i < len; i++)
            {
                if (args[i] == NULL)
                {
                    perror("Memory allocation for strings failed");
                    return 1;
                }
            }

            // ensure the last element of the argsTest array is NULL
            argsTest[len] = NULL;

            // printf("argsTest[%d]: %s\n", len, argsTest[len]);
            // printf("Printing argsTest:\n");

            // printing argsTest size
            // for (int i = 0; argsTest[i] != NULL; i++)
            // {
            //     printf("%s\n", argsTest[i]);
            // }

            // int argsTestSize = sizeof(argsTest) / sizeof(argsTest[0]);
            // printf("Number of arguments in argsTest (after adding NULL - last index): %d\n", argsTestSize);

            // * EXECUTE COMMANDS
            printf("START EXECUTION USER COMMAND\n");
            pid_t child_pid = fork(); // create a new process (child).

            if (child_pid == 0) // fork succeed to create a child process
            {
                // child process
                execvp(command, argsTest); // execute the specified command with given arguments

                // if execvp() fails, it means the command doesn't exist
                fprintf(stderr, "Command '%s' not found\n", command);
                // if execvp() fails, handle the error
                perror("execvp"); // print an error message
                _exit(1);         // exit the child process with a non-zero status to indicate an error
            }
            else if (child_pid < 0) // fork failed to create a child process
            {
                perror("fork"); // print an error message
            }
            else // parent process
            {
                int status;                     // variable to store the child process's exit status
                waitpid(child_pid, &status, 0); // wait for the child process to complete

                // print the child PID and return result (exit status)
                printf("Child PID: %d\n", child_pid);
                printf("Return Result: %d\n", status);
            }
            printf("END EXECUTION USER COMMAND\n");

            free(argsTest); // free allocated memory
        }
    }
}