#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argumentCount, char *argumentValues[])
{
    // * READ USER INPUT
    char input[1024]; // store user input command of 1024 characters

    while (1) // infinite loop
    {
        printf("YourShell> ");              // indicate shell ready to accept user input
        fgets(input, sizeof(input), stdin); // read the user input + store in input

        printf("Input size: %zu\n", strlen(input));
        printf("Input size (excluding newline): %zu\n", strlen(input) - 1);

        // remove trailing newline character if present
        if (input[strlen(input) - 1] == '\n') // check if last char in input is a newline character
        {
            input[strlen(input) - 1] = '\0'; // replace '\n' with null terminator '\0'
        }
    }

    // * PARSE USER INPUT

    // * EXECUTE COMMANDS
}