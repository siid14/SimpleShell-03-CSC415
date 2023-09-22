#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#define MAX_COMMANDS 32
#define MAX_COMMAND_LENGTH 100

struct Command
{
    char executable[MAX_COMMAND_LENGTH];
    char *arguments[MAX_COMMANDS];
    int argumentCount;
};

struct Command commands[MAX_COMMANDS];
int commandCount = 0;

void executeCommand(struct Command *cmd, int prevPipe[2])
{
    pid_t child_pid = fork();

    if (child_pid == 0)
    {
        // Child process
        if (prevPipe != NULL)
        {
            close(prevPipe[1]);              // Close the write end of the previous pipe
            dup2(prevPipe[0], STDIN_FILENO); // Redirect stdin from the previous pipe
            close(prevPipe[0]);              // Close the read end of the previous pipe
        }

        execvp(cmd->executable, cmd->arguments); // Execute the command
        perror("execvp");                        // Handle execvp error
        exit(1);                                 // Exit the child process with an error status
    }
    else if (child_pid < 0)
    {
        perror("fork"); // Handle fork error
    }
}

int main(int argc, char *argv[])
{
    char input[187];

    while (1)
    {
        printf("YourShell> ");
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            if (feof(stdin))
            {
                printf("EOF error - Exiting the shell\n");
                return 0;
            }
            else
            {
                perror("fgets");
                exit(1);
            }
        }

        if (strlen(input) == 1)
        {
            printf("Empty string entered, please enter a valid string\n");
            perror("Empty string error");
            continue;
        }

        char *token = strtok(input, "|");
        while (token != NULL)
        {
            if (commandCount < MAX_COMMANDS)
            {
                struct Command *cmd = &commands[commandCount];
                cmd->argumentCount = 0;
                char *argToken = strtok(token, " ");
                strncpy(cmd->executable, argToken, sizeof(cmd->executable));
                cmd->arguments[0] = cmd->executable;
                cmd->argumentCount++;

                while (argToken != NULL && cmd->argumentCount < MAX_COMMANDS - 1)
                {
                    argToken = strtok(NULL, " ");
                    if (argToken != NULL)
                    {
                        cmd->arguments[cmd->argumentCount] = argToken;
                        cmd->argumentCount++;
                    }
                }
                commandCount++;
            }
            else
            {
                printf("Too many commands. Maximum allowed: %d\n", MAX_COMMANDS);
                break;
            }

            token = strtok(NULL, "|");
        }

        int prevPipe[2] = {0, 1}; // Initialize a pipe for the first command

        for (int i = 0; i < commandCount; i++)
        {
            struct Command *cmd = &commands[i];
            executeCommand(cmd, prevPipe);

            // Close the read end of the previous pipe
            if (prevPipe[0] != 0)
            {
                close(prevPipe[0]);
            }

            // Create a new pipe for the next command (except for the last command)
            if (i < commandCount - 1)
            {
                int newPipe[2];
                if (pipe(newPipe) == -1)
                {
                    perror("pipe");
                    exit(1);
                }
                prevPipe[0] = newPipe[0]; // Set the read end of the new pipe as the previous pipe
                prevPipe[1] = newPipe[1]; // Set the write end of the new pipe as the previous pipe
            }
        }

        // Wait for the last command to finish
        int status;
        wait(&status);

        // Reset commandCount for the next iteration
        commandCount = 0;
    }

    return 0;
}
