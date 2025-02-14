#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"
#include "interpreter.h"
#include "shellmemory.h"

int parseInput (char ui[]);

// Start of everything
int main (int argc, char *argv[]) {
    printf ("Shell version 1.4 created December 2024\n");

    char prompt = '$';          // Shell prompt
    char userInput[MAX_USER_INPUT];     // user's input stored here
    int errorCode = 0;          // zero means no error, default

    //init user input
    for (int i = 0; i < MAX_USER_INPUT; i++) {
        userInput[i] = '\0';
    }

    int interactive = isatty (STDIN_FILENO);

    //init shell memory
    mem_init ();
    while (1) {
        if (interactive) {
            printf ("%c ", prompt);
            fflush (stdout);
        }
        // here you should check the unistd library 
        // so that you can find a way to not display $ in the batch mode
        if (fgets (userInput, MAX_USER_INPUT - 1, stdin) == NULL) {
            break;
        }
        errorCode = parseInput (userInput);
        if (errorCode == -1)
            exit (99);          // ignore all other errors
        memset (userInput, 0, sizeof (userInput));
    }

    return 0;
}

int wordEnding (char c) {
    // You may want to add ';' to this at some point,
    // or you may want to find a different way to implement chains.
    return c == '\0' || c == '\n' || c == ' ' || c == ';';
}

int parseInput (char inp[]) {
    char tmp[200], *words[100];
    int ix = 0, w = 0;
    int wordlen;
    int errorCode;

    // iterate through instruction(s)
    while (inp[ix] != '\n' && inp[ix] != '\0' && ix < 1000) {
        // skip white spaces
        while (inp[ix] == ' ' && ix < 1000) {
            ix++;
        }

        // check for end of instruction
        if (inp[ix] == '\n' || inp[ix] == '\0') {
            break;
        }
        // extract a word
        for (wordlen = 0; !wordEnding (inp[ix]) && ix < 1000; ix++, wordlen++) {
            tmp[wordlen] = inp[ix];
        }

        // store word
        tmp[wordlen] = '\0';
        words[w] = strdup (tmp);
        w++;

        // check for end of chained instruction
        if (inp[ix] == ';') {
            errorCode = interpreter (words, w);
            if (errorCode == -1) {
                return -1;      // exit on error
            }
            w = 0;
        }

        ix++;
    }
    errorCode = interpreter (words, w);
    return errorCode;
}
