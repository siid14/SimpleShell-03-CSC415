#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> // include for pid_t
#include <sys/wait.h>  // include for waitpid
#include <unistd.h>    // include for fork and execvp
#include <errno.h>     // include errno to access the error code

int main(int argumentCount, char *argumentValues[])
{
    // * READ USER INPUT
    char input[1024]; // store user input command of 1024 characters

    while (1) // infinite loop
    {
        printf("YourShell> ");              // indicate shell ready to accept user input
        fgets(input, sizeof(input), stdin); // read the user input + store in input

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            // check if fgets encountered EOF (Ctrl+D)
            if (feof(stdin))
            {
                // gracefully exit on EOF without reporting an error
                printf("Exiting the shell.\n");
                break;
            }
            else
            {
                // handle and report other input errors
                perror("fgets");
                exit(1); // exit the shell on error
            }
        }

        printf("Input size (including newline): %zu\n", strlen(input));
        printf("Input size (excluding newline): %zu\n", strlen(input) - 1);

        // remove trailing newline character if present
        if (input[strlen(input) - 1] == '\n') // check if last char in input is a newline character
        {
            input[strlen(input) - 1] = '\0'; // replace '\n' with null terminator '\0'
        }
    }

    // * PARSE USER INPUT
    char *command = strtok(input, " "); // tokenize first string of user input
    printf("Command: %s\n", command);

    char *args[32];
    // ** store remaining input string (arguments) into args
    int arg_count = 0;

    while ((args[arg_count] = strtok(NULL, " "))) // tokenize each remaining string of input
                                                  // until no more found
    {
        arg_count++;
    }
    printf("Number of argument in args: %d\n", arg_count);

    // * EXECUTE COMMANDS
    pid_t child_pid = fork(); // create a new process (child).

    if (child_pid == 0) // fork succeed to create a child process
    {
        // child process
        execvp(command, args); // execute the specified command with given arguments

        // if execvp() fails, handle the error
        perror("execvp"); // print an error message
        exit(1);          // exit the child process with a non-zero status to indicate an error
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
}