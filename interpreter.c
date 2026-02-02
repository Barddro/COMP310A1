#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "shellmemory.h"
#include "shell.h"

int MAX_ARGS_SIZE = 3; // defines maximum argument size

// helper function for interpreter to indicate invalid command
int badcommand() {
    printf("Unknown Command\n");
    return 1;
}

// for source command only
int badcommandFileDoesNotExist() {
    printf("Bad command: File not found\n");
    return 3;
}

// function prototypes for interpreter() function
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
int run(char* words[]);
int badcommandFileDoesNotExist();

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size) {
    int i;

    if (args_size < 1 || args_size > MAX_ARGS_SIZE) { // invalid argument size
        return badcommand();
    }

    for (i = 0; i < args_size; i++) {   // terminate args at newlines
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0) { // help command
        if (args_size != 1)
            return badcommand();
        return help();

    } else if (strcmp(command_args[0], "quit") == 0) { // quit command
        if (args_size != 1)
            return badcommand();
        return quit();

    } else if (strcmp(command_args[0], "set") == 0) { // set command
        if (args_size != 3)
            return badcommand();
        return set(command_args[1], command_args[2]);

    } else if (strcmp(command_args[0], "print") == 0) { // print command
        if (args_size != 2)
            return badcommand();
        return print(command_args[1]);

    } else if (strcmp(command_args[0], "source") == 0) { // source command
        if (args_size != 2)
            return badcommand();
        return source(command_args[1]);

    } else if (strcmp(command_args[0], "echo") == 0) { // echo command
        if (args_size != 2) 
            return badcommand();
        return echo(command_args[1]);

    } else if (strcmp(command_args[0], "my_ls") == 0) { // my_ls command
        if (args_size != 1)
            return badcommand();
        return my_ls();

    } else if (strcmp(command_args[0], "my_mkdir") == 0) { // my_mkdir command
        if (args_size != 2)
            return badcommand();
        return my_mkdir(command_args[1]);

    } else if (strcmp(command_args[0], "my_touch") == 0) { // my_touch command
        if (args_size != 2) 
            return badcommand();
        return my_touch(command_args[1]);

    } else if (strcmp(command_args[0], "my_cd") == 0) { // my_cd command
        if (args_size != 2) 
            return badcommand();
        return my_cd(command_args[1]);

    } else if (strcmp(command_args[0], "run") == 0) { // run command
        // create copy of command_args without 'run'
        char* args[args_size];
        for (int i = 1; i < args_size; i++) {
            args[i - 1] = command_args[i];
        }
        args[args_size - 1] = NULL;
        
        int errorCode = run(args); // use args as parameter for run()

        return errorCode;
        
    } else { // invalid command
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

// prints input to shell
int echo(char* s) {

    if (s[0] == '$') { // do variable substitution if arg begins with '$'
        char* resp = mem_get_value(s+1); // response string
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

// prints subdirectories and files within current directory
int my_ls() {

    struct dirent **namelist; // array of directory entries returned by scandir

    int n = scandir(".", &namelist, 0, alphasort); // puts subdirectory and file names in namelist (in alphanumeric sorted order), returns number of entries found
    if (n < 0) { // no entries found
        perror("scandir");
    } else {
        for (int i = 0; i < n; i++) {
            printf("%s\n", namelist[i]->d_name); //prints d_name field of namelist[i]
            free(namelist[i]);
        }
    }
    free(namelist);

    return 0;
}

// Helper function to check if a given string is alphanumeric
int isAlphanumeric(char* str) {
    for(int i = 0; i < strlen(str); i++) {
        if (!isalnum(str[i])) {
            return 0;
        }
    }
    return 1;
}

// creates a new directory within current directoy
int my_mkdir(char *dirname) {
    char *name = ""; // name of directory you want to make
    int need_free = 0; // indicates if we need to free allocated string 'name'

    // check if argument is variable
    if (dirname[0] == '$') {
        name = mem_get_value(dirname + 1);

        // variable doesn't exist in shell memory
        if (strcmp(name, "Variable does not exist") == 0 || !isAlphanumeric(name)) {
            printf("Bad command: my_mkdir\n");
            return 1;
        }
        
        // here, we make a copy of variable in memory as a string (name), 
        // so we must remember to free this allocated space
        need_free = 1;

    } else {
        
        if (!isAlphanumeric(dirname)) { // check if given directory name is not alphanumeric
            printf("Bad command: my_mkdir\n");
            return 1;
        }
        name = dirname;

    }

    if (mkdir(name, 0755) == -1) { // 0755 --> standard permissions for new directory
        // mkdir returns -1 on error
        perror("mkdir failed");
        if (need_free) free(name);
        return 1;
    }

    if (need_free) free(name);
    return 0;
}

// creates a new file in current directory
int my_touch (char* filename) {

    FILE *fptr;

    // Create a file
    fptr = fopen(filename, "w");

    // Close the file
    fclose(fptr);

    return 0;
}

// sets current directory to 'dirname'
int my_cd (char* dirname) {

    if (chdir(dirname) == 0) {
        return 0;
    }
    printf("Bad command: my_cd\n");
    return 1;
}

// runs specified command in a new shell
int run(char* words[]) {

    pid_t pid = fork(); // fork to create a child process to exec command in
    if (pid != 0) { // pid == 0 if process is a child, so we only wait if process is parent
        wait(NULL);
    } else { // otherwise, we use execvp to run a command passed in as the individual 'words' that make up that command
        execvp(words[0], words);
    }
    
    return 0;
}


