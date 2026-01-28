#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "shellmemory.h"
#include "shell.h"

int MAX_ARGS_SIZE = 3;

int badcommand() {
    printf("Unknown Command\n");
    return 1;
}

// For source command only
int badcommandFileDoesNotExist() {
    printf("Bad command: File not found\n");
    return 3;
}


int help();
int quit();
int set(char *var, char *value);
int print(char *var);
int source(char *script);
int echo(char* s);
int my_ls();
int my_mkdir(char* dirname);
int my_touch(char* filename);
int my_cd(char* dirname);
int badcommandFileDoesNotExist();

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size) {
    int i;

    if (args_size < 1 || args_size > MAX_ARGS_SIZE) {
        return badcommand();
    }

    for (i = 0; i < args_size; i++) {   // terminate args at newlines
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0) {
        //help
        if (args_size != 1)
            return badcommand();
        return help();

    } else if (strcmp(command_args[0], "quit") == 0) {
        //quit
        if (args_size != 1)
            return badcommand();
        return quit();

    } else if (strcmp(command_args[0], "set") == 0) {
        //set
        if (args_size != 3)
            return badcommand();
        return set(command_args[1], command_args[2]);

    } else if (strcmp(command_args[0], "print") == 0) {
        //print
        if (args_size != 2)
            return badcommand();
        return print(command_args[1]);

    } else if (strcmp(command_args[0], "source") == 0) {
        //source
        if (args_size != 2)
            return badcommand();
        return source(command_args[1]);

    } else if (strcmp(command_args[0], "echo") == 0) {
        //echo
        if (args_size != 2) {
            return badcommand();
        }
        return echo(command_args[1]);

    } else if (strcmp(command_args[0], "my_ls") == 0) {
        //my_ls
        if (args_size != 1) {
            return badcommand();
        }
        return my_ls();

    } else if (strcmp(command_args[0], "my_mkdir") == 0) {
        //my_mkdir
        if (args_size != 2) {
            return badcommand();
        }
        return my_mkdir(command_args[1]);

    } else if (strcmp(command_args[0], "my_touch") == 0) {
        //my_touch
        if (args_size != 2) {
            return badcommand();
        }
        return my_touch(command_args[1]);

    } else if (strcmp(command_args[0], "my_cd") == 0) {
        //my_cd
        if (args_size != 2) {
            return badcommand();
        }
        return my_cd(command_args[1]);
    }



    else {
        return badcommand();
    }
}

int help() {

    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
source SCRIPT.TXT	Executes the file SCRIPT.TXT\n ";
    printf("%s\n", help_string);
    return 0;
}

int quit() {
    printf("Bye!\n");
    exit(0);
}

int set(char *var, char *value) {
    // Challenge: allow setting VAR to the rest of the input line,
    // possibly including spaces.

    // Hint: Since "value" might contain multiple tokens, you'll need to loop
    // through them, concatenate each token to the buffer, and handle spacing
    // appropriately. Investigate how `strcat` works and how you can use it
    // effectively here.

    mem_set_value(var, value);
    return 0;
}


int print(char *var) {
    printf("%s\n", mem_get_value(var));
    return 0;
}

int source(char *script) {
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(script, "rt");      // the program is in a file

    if (p == NULL) {
        return badcommandFileDoesNotExist();
    }

    fgets(line, MAX_USER_INPUT - 1, p);
    while (1) {
        errCode = parseInput(line);     // which calls interpreter()
        memset(line, 0, sizeof(line));

        if (feof(p)) {
            break;
        }
        fgets(line, MAX_USER_INPUT - 1, p);
    }

    fclose(p);

    return errCode;
}

int echo(char* s) {

    if (s[0] == '$') {
        char* resp = mem_get_value(s+1);
        if (strcmp(resp, "Variable does not exist") == 0) {
            printf("\n");
        } else {
            printf("%s\n", resp);
            free(resp);
        }
    } else {
        printf("%s\n", s);
    }
    return 0;
}

//Helper function for comparing strings - used in my_ls()
int compare_strings(const void *a, const void *b) {
    // Cast the void pointers to char pointers (or char array pointers)
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    // Use strcmp to compare the actual strings
    return strcmp(*ia, *ib);
}

int my_ls() {

    DIR *dir = opendir("."); //open current directory

    char* entries[1024];
    int index = 0;

    while(1) {
        struct dirent *entry = readdir(dir);
        if (entry != NULL) { 
            entries[index++] = strdup(entry->d_name); //add entry names to entries
        } else { //no more entries
            break;
        }
    }

    qsort(entries, index, sizeof(char*), compare_strings); //sort entries array

    for (int i = 0; i < index; i++) {
        printf("%s\n", entries[i]);
        free(entries[i]);
    }

    closedir(dir);

    return 0;
}

//Helper function to check alphanumeric strings
int isAlphanumeric(char* str) {
    for(int i = 0; i < strlen(str); i++) {
        if (!isalnum(str[i])) {
            return 0;
        }
    }
    return 1;
}

int my_mkdir(char *dirname) {
    char *name = "";
    int need_free = 0; //indicates if we need to free name

    if (dirname[0] == '$') {
        name = mem_get_value(dirname + 1);

        if (strcmp(name, "Variable does not exist") == 0 || !isAlphanumeric(name)) {
            printf("Bad command: my_mkdir\n");
            return 1;
        }

        need_free = 1;

    } else {
        if (!isAlphanumeric(dirname)) {
            printf("Bad command: my_mkdir\n");
            return 1;
        }
        name = dirname;
    }

    if (mkdir(name, 0755) == -1) {
        perror("mkdir failed");
        if (need_free) free(name);
        return 1;
    }

    if (need_free) free(name);
    return 0;
}

int my_touch (char* filename) {

    FILE *fptr;

    // Create a file
    fptr = fopen(filename, "w");

    // Close the file
    fclose(fptr);

    return 0;
}

int my_cd (char* dirname) {

    return 0;
}

