#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shellmemory.h"
#include "shell.h"
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>  
#include <unistd.h>

int MAX_ARGS_SIZE = 3;

int badcommand () {
    printf ("Unknown Command\n");
    return 1;
}

// For source command only
int badcommandFileDoesNotExist () {
    printf ("Bad command: File not found\n");
    return 3;
}

int help ();
int quit ();
int set (char *var, char *value);
int print (char *var);
int source (char *script);
int echo(char *arg);
int my_ls(void);
int my_mkdir(char *dirname);
int my_touch(char *filename);
int my_cd(char *dirname);
int badcommandFileDoesNotExist ();

// Interpret commands and their arguments
int interpreter (char *command_args[], int args_size) {
    int i;

    if (args_size < 1 || args_size > MAX_ARGS_SIZE) {
        return badcommand ();
    }

    for (i = 0; i < args_size; i++) {   // terminate args at newlines
        command_args[i][strcspn (command_args[i], "\r\n")] = 0;
    }

    if (strcmp (command_args[0], "help") == 0) {
        //help
        if (args_size != 1)
            return badcommand ();
        return help ();

    } else if (strcmp (command_args[0], "quit") == 0) {
        //quit
        if (args_size != 1)
            return badcommand ();
        return quit ();

    } else if (strcmp (command_args[0], "set") == 0) {
        //set
        if (args_size != 3)
            return badcommand ();
        return set (command_args[1], command_args[2]);

    } else if (strcmp (command_args[0], "print") == 0) {
        if (args_size != 2)
            return badcommand ();
        return print (command_args[1]);

    } else if (strcmp (command_args[0], "source") == 0) {
        if (args_size != 2)
            return badcommand ();
        return source (command_args[1]);

    } else if (strcmp (command_args[0], "echo") == 0) {
        if (args_size != 2)
            return badcommand ();
        return echo(command_args[1]);
    } else if (strcmp(command_args[0], "my_ls") == 0) {
        if (args_size != 1)
            return badcommand();
        return my_ls();
    } else if (strcmp(command_args[0], "my_mkdir") == 0) {
        if (args_size != 2)
            return badcommand();
        return my_mkdir(command_args[1]);
    } else if (strcmp(command_args[0], "my_touch") == 0) {
        if (args_size != 2)
            return badcommand();
        return my_touch(command_args[1]);
    } else if (strcmp(command_args[0], "my_cd") == 0) {
        if (args_size != 2)
            return badcommand();
        return my_cd(command_args[1]);
    } else
        return badcommand ();
}

int help () {

    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
source SCRIPT.TXT	Executes the file SCRIPT.TXT\n ";
    printf ("%s\n", help_string);
    return 0;
}

int quit () {
    printf ("Bye!\n");
    exit (0);
}

int set (char *var, char *value) {
    char *link = " ";

    // Hint: If "value" contains multiple tokens, you'll need to loop through them, 
    // concatenate each token to the buffer, and handle spacing appropriately. 
    // Investigate how `strcat` works and how you can use it effectively here.

    mem_set_value (var, value);
    return 0;
}


int print (char *var) {
    printf ("%s\n", mem_get_value (var));
    return 0;
}

int source (char *script) {
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen (script, "rt");     // the program is in a file

    if (p == NULL) {
        return badcommandFileDoesNotExist ();
    }

    fgets (line, MAX_USER_INPUT - 1, p);
    while (1) {
        errCode = parseInput (line);    // which calls interpreter()
        memset (line, 0, sizeof (line));

        if (feof (p)) {
            break;
        }
        fgets (line, MAX_USER_INPUT - 1, p);
    }

    fclose (p);

    return errCode;
}

int echo(char *arg) {
    if (arg[0] == '$') {
        char *varName = arg + 1;
        char *value = mem_get_value(varName);
        if (value != NULL && strcmp(value, "Variable does not exist") != 0)
            printf("%s\n", value);
        else
            printf("\n");
    } else {
        printf("%s\n", arg);
    }
    return 0;
}

/* Helper:
 * This comparator ensures:
 *  - Names starting with a number will appear before lines starting with a letter.
    - Names starting with a letter that appears earlier in the alphabet will appear before lines starting with a letter that appears later in the alphabet.
    - Names starting with an uppercase letter will appear before lines starting with the same letter in lowercase.	
 *  - Other characters will be sorted by their ASCII value.
 */
int my_compare(const void *a, const void *b) {
    const char *s1 = *(const char **) a;
    const char *s2 = *(const char **) b;
    int i = 0;

    // Character by character.
    while (s1[i] && s2[i]) {
        if (s1[i] == s2[i]) {
            i++;
            continue;
        }
        // see if digits or letters.
        int isDigit1 = isdigit((unsigned char)s1[i]);
        int isDigit2 = isdigit((unsigned char)s2[i]);
        int isAlpha1 = isalpha((unsigned char)s1[i]);
        int isAlpha2 = isalpha((unsigned char)s2[i]);

        // digit comes first.
        if (isDigit1 && isAlpha2)
            return -1;
        if (isAlpha1 && isDigit2)
            return 1;

        // If both are letters, compare in a case-insensitive manner first.
        if (isAlpha1 && isAlpha2) {
            char lower1 = tolower(s1[i]);
            char lower2 = tolower(s2[i]);
            if (lower1 == lower2) {
                // uppercase comes before lowercase.
                if (s1[i] != s2[i]) {
                    return (s1[i] < s2[i]) ? -1 : 1;
                }
            } else {
                return (lower1 < lower2) ? -1 : 1;
            }
        }
        // For other cases, use normal ASCII ordering.
        return (s1[i] < s2[i]) ? -1 : 1;
    }
    // If one string ended, the shorter one comes first.
    if (s1[i] == s2[i])
        return 0;
    return (s1[i] == '\0') ? -1 : 1;
}

int my_ls() {
    DIR *dp;
    struct dirent *entry;
    char *filenames[1024];
    int count = 0;
    
    dp = opendir(".");
    if (dp == NULL) {
        return -1;
    }
    
    // Read all directory entries.
    while ((entry = readdir(dp)) != NULL) {
        filenames[count] = strdup(entry->d_name);
        count++;
    }
    closedir(dp);
    
    // Sort the names using comparator.
    qsort(filenames, count, sizeof(char *), my_compare);
    
    // Print each name on a separate line.
    for (int i = 0; i < count; i++) {
        printf("%s\n", filenames[i]);
        free(filenames[i]);
    }
    
    return 0;
}

// Helper: returns 1 if str consists only of alphanumeric characters.
int isAlphanumString(const char *str) {
    if (str == NULL || *str == '\0')
        return 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isalnum((unsigned char)str[i]))
            return 0;
    }
    return 1;
}

int my_mkdir(char *dirname) {
    char actualName[100];

    if (dirname[0] == '$') {
        char *varName = dirname + 1;
        char *value = mem_get_value(varName);
        if (value == NULL || strcmp(value, "Variable does not exist") == 0 || !isAlphanumString(value)) {
            printf("Bad command: my_mkdir\n");
            return 1;
        }        
        strncpy(actualName, value, sizeof(actualName));
        actualName[sizeof(actualName) - 1] = '\0';
    } else {
        if (!isAlphanumString(dirname)) {
            printf("Bad command: my_mkdir\n");
            return 1;
        }
        strncpy(actualName, dirname, sizeof(actualName));
        actualName[sizeof(actualName) - 1] = '\0';
    }

    if (mkdir(actualName, 0777) != 0) {
        return 1;
    }
    return 0;
}

int my_touch(char *filename) {
    if (!isAlphanumString(filename)) {
        return 1;
    }
    
    // will create the file if does not exist
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        return 1;
    }
    fclose(fp);
    return 0;
}

int my_cd(char *dirname) {
    if (!isAlphanumString(dirname)) {
        printf("Bad command: my_cd\n");
        return 1;
    }
    
    if (chdir(dirname) != 0) {
        printf("Bad command: my_cd\n");
        return 1;
    }
    return 0;
}
