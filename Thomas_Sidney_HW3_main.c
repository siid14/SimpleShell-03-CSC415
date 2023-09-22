/**************************************************************
 * Class:  CSC-415-01 Fall 2023
 * Name: Sidney Thomas
 * Student ID: 918656419
 * GitHub ID: siid14
 * Project: Assignment 3 – Simple Shell
 *
 * File: Thomas_Sidney_HW3_main.c
 *
 * Description:  The provided code is a C program for a simple shell.
 *               It allows users to input and execute command-line commands.
 *               The code features command parsing, execution,
 *               and piping support, along with error handling.
 *               Users can gracefully exit the shell using the "exit" command.
 *               The program provides user-friendly prompts,
 *               handles empty input, and exits gracefully on encountering EOF.
 **************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> // include for pid_t
#include <sys/wait.h>  // include for waitpid
#include <unistd.h>    // include for fork and execvp
#include <errno.h>     // include errno to access the error code
#include <stdbool.h>   // include this header for bool type
#include <ctype.h>

// function to check if the input is empty or contains only whitespace
bool isWhiteSpaceInput(const char *input)
{
    // iterate through each character in the input string
    for (int i = 0; input[i] != '\0'; i++)
    {
        // check if the character is not a whitespace character
        if (!isspace(input[i]))
        {
            return false; // if any non-Wspace character is found
        }
    }
    return true; // if all characters are whitespace
}

#define MAX_COMMANDS 10  // maximum number of commands in a pipeline
#define MAX_ARGUMENTS 32 // maximum number of arguments

int main(int argumentCount, char *argumentValues[])
{

    char input[187];                // store u-input command of 187 characters
    char *saveptr;                  // save pointer for strtok_r
    int pipes[MAX_COMMANDS - 1][2]; // array to store pipe file descriptors

    // * READ USER INPUT
    while (1) // infinite loop
    {
        printf("YourShell> "); // indicate shell ready to accept u-input

        // read user input + handle errors
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            // check if fgets encountered EOF
            if (feof(stdin))
            {
                // gracefully exit on EOF without reporting an error
                printf("EOF error - Exiting the shell\n");
                return 0;
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

        // check for whitespace input
        if (isWhiteSpaceInput(input))
        {
            printf("Blank line entered, please enter a valid command\n");
            continue;
        }

        // check for pipes in user input
        char *pipeCMD = strchr(input, '|');

        // * HANDLE USER INPUT IF IT HAS PIPING OR NOT
        if (pipeCMD) // handle piping logic
        {
            // * PARSE USER INPUT
            // split input into commands based on '|'
            char *command = strtok_r(input, "|", &saveptr);
            int pipeIndex = 0; // initialize the pipe index
            while (command != NULL)
            {

                // create a pipe for the next pair of commands (if applicable)
                if (pipeIndex < MAX_COMMANDS - 1)
                {
                    if (pipe(pipes[pipeIndex]) == -1)
                    {
                        perror("pipe");
                        exit(1);
                    }
                }

                // * EXECUTE COMMANDS
                // fork a child process for the current command
                pid_t childPID = fork();
                if (childPID == -1)
                {
                    perror("fork");
                    exit(1);
                }

                if (childPID == 0)
                {
                    // child process
                    if (pipeIndex > 0)
                    {
                        // redirect standard input (stdin)
                        // to the read end of the previous pipe
                        dup2(pipes[pipeIndex - 1][0], 0);
                        // close the read end of the previous pipe
                        close(pipes[pipeIndex - 1][0]);
                    }

                    if (pipeIndex < MAX_COMMANDS - 1)
                    {
                        // redirect standard output (stdout)
                        // to the write end of the current pipe
                        dup2(pipes[pipeIndex][1], 1);
                        // close the write end of the current pipe
                        close(pipes[pipeIndex][1]);
                    }

                    // close all other pipe ends in the child process
                    for (int i = 0; i < MAX_COMMANDS - 1; i++)
                    {
                        if (i != pipeIndex - 1 && i != pipeIndex)
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
                else // parent process
                {

                    // close pipe ends in the parent process
                    if (pipeIndex > 0)
                    {
                        close(pipes[pipeIndex - 1][0]);
                        close(pipes[pipeIndex - 1][1]);
                    }

                    pipeIndex++;
                }

                // move to the next command
                command = strtok_r(NULL, "|", &saveptr);
            }

            // close unused pipe ends
            for (int i = 0; i < pipeIndex; i++)
            {
                // close the write end of the previous pipe
                close(pipes[i][1]);

                // close the read end of the current pipe
                close(pipes[i][0]);
            }
        }
        else // handle non-piping logic
        {
            // * PARSE USER INPUT
            // remove trailing newline character if present
            // check if last char in input is a newline character
            if (input[strlen(input) - 1] == '\n')
            {
                // replace '\n' with null terminator '\0'
                input[strlen(input) - 1] = '\0';
            }

            // tokenize first string of user input
            char *command = strtok(input, " ");

            // handle exit input to exit shell (edge case)
            if (strcmp(command, "exit") == 0 || strcmp(command, "EXIT") == 0)
            {
                printf("User type 'exit' - Exiting the shell\n");
                break; // exit the loop and terminate the shell
            }

            char *argsA[32];
            // * store remaining input string (arguments) into argsA
            int argsCountA = 0;

            bool vectorOverrun = false;
            // tokenize each remaining string of input until no more found
            while ((argsA[argsCountA] = strtok(NULL, " ")))
            {
                argsCountA++;

                // check for argument vector overrun
                if (argsCountA >= MAX_ARGUMENTS)
                {
                    vectorOverrun = true;
                }
            }

            // gracefully exit if too many arguments
            if (vectorOverrun)
            {
                fprintf(stderr, "vector overrun: Too many arguments\n");
                break;
            }

            char **argsB;
            int len = argsCountA; // number of strings to store

            argsB = malloc(sizeof(char *) * len);

            // check if memory allocation was successful
            if (argsB == NULL)
            {
                perror("Memory allocation failed");
                free(argsB); // free the allocated memory
                return 1;
            }

            // copy argsA content into argsB
            for (int i = 0; i < len; i++)
            {
                argsB[i] = argsA[i];
            }

            // check if memory allocation for strings was successful
            for (int i = 0; i < len; i++)
            {
                if (argsA[i] == NULL)
                {
                    perror("Memory allocation for strings failed");
                    return 1;
                }
            }

            // ensure the last element of the argsB array is NULL
            argsB[len] = NULL;

            // * EXECUTE COMMANDS
            pid_t childPID = fork(); // create a new process (child).

            if (childPID == 0) // fork succeed to create a child process
            {
                // child process
                // execute the specified command with given arguments
                execvp(command, argsB);

                // if execvp() fails, it means the command doesn't exist
                fprintf(stderr, "Command '%s' not found\n", command);
                // if execvp() fails, handle the error
                perror("execvp"); // print an error message
                // exit the child process
                // with a non-zero status to indicate an error
                _exit(1);
            }
            else if (childPID < 0) // fork failed to create a child process
            {
                perror("fork"); // print an error message
            }
            else // parent process
            {
                // store the child process's exit status
                int status;
                // wait for the child process to complete
                waitpid(childPID, &status, 0);

                // print the child PID and return result (exit status)
                printf("Child PID: %d\n", childPID);
                printf("Return Result: %d\n", status);
            }
            free(argsB); // free allocated memory
        }
    }
}