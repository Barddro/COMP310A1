#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include "shell.h"
#include "interpreter.h"
#include "shellmemory.h"

int parseInput(char ui[]);

// Start of everything
int main(int argc, char *argv[]) {
    printf("Shell version 1.5 created Dec 2025\n");

    char prompt = '$';  				// Shell prompt
    char userInput[MAX_USER_INPUT];		// user's input stored here
    int errorCode = 0;					// zero means no error, default

    // init user input
    for (int i = 0; i < MAX_USER_INPUT; i++) {
        userInput[i] = '\0';
    }
    
    // init shell memory
    mem_init();
    while(1) {
    
        if (isatty(STDIN_FILENO)) { // isatty returns 1 for interactive mode, 0 for batch mode
            printf("%c ", prompt); // print shell prompt
        }   

        if (fgets(userInput, MAX_USER_INPUT-1, stdin) == NULL) {
            break;
        }
        errorCode = parseInput(userInput);
        if (errorCode == -1) exit(99);	// ignore all other errors
        memset(userInput, 0, sizeof(userInput));
    }

    return 0;
}

// helper function for parseInput that checks if a word is ending
int wordEnding(char c) {
    // added ';' to list of word endings to support one-liners
    return c == '\0' || c == '\n' || c == ' ' || c == ';';
}

// parseInput parses the input to terminal and passes it into the interpreter
// Returns an error code if interpreter fails
int parseInput(char inp[]) {

    char tmp[200], *words[100]; // tmp - buffer for one word, words - array of words                         
    int ix = 0;                 // index for input string
    int w = 0;                  // index of the current word in 'words
    int wordlen;                // index of current character in current word
    int errorCode;              // return error code from interpreter

    // loop over entire inputted command/string
    while (inp[ix] != '\n' && inp[ix] != '\0' && ix < 1000) {
        for (ix; inp[ix] == ' ' && ix < 1000; ix++); // skip white spaces

        // extract a word
        for (wordlen = 0; !wordEnding(inp[ix]) && ix < 1000; ix++, wordlen++) {
            tmp[wordlen] = inp[ix];                        
        }

        tmp[wordlen] = '\0'; // set null terminator 
        words[w] = strdup(tmp); // copy the current word of command into 'words'
        w++;

        if (inp[ix] == '\0') { // if inputted string terminates, stop parsing completely
            break;
        } else if (inp[ix] == ';') { // if we reach ';'...
            errorCode = interpreter(words, w); //run current comand collected in words
            for (int i = 0; i < w; i++) { // free all words in 'words'
                free(words[i]); 
            }
            w = 0; // reset index of 'words' for next command to be parsed
        }
        ix++; 
    }
    // this makes a call back to interpreter with new line
    // can either fix by exiting if eof, or returning a list of commands to run
    errorCode = interpreter(words, w);
    
    for (int i = 0; i < w; i++) { // free all words in 'words'
        free(words[i]);
    }

    return errorCode; // return error code if produced
}