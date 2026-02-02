#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shellmemory.h"
#include "shell.h"
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

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
int run(char *command_args[], int num_of_args);
int help();
int quit();
int set(char *var, char *value);
int print(char *var);
int source(char *script);
int badcommandFileDoesNotExist();

bool isalphanumeric (const char *str) {
	for (int i = 0; str[i] != '\0'; i ++) {
		if (!isalnum((unsigned char) str[i])){
			return false;
		}
	}
	return true;
}

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
        if (args_size != 2)
            return badcommand();
        return print(command_args[1]);

    } else if (strcmp(command_args[0], "source") == 0) {
        if (args_size != 2)
            return badcommand();
        return source(command_args[1]);

    } else if (strcmp(command_args[0], "echo") == 0) {
	if (args_size != 2)
	    return badcommand();
	//printf("%c",command_args[1][0]);
	if (isalphanumeric(command_args[1])){
	    printf("%s", command_args[1]);
	    printf("\n");
	    return 0;}
	else if (command_args[1][0] == '$' && isalphanumeric(command_args[1]+1) ){	
	   return print(command_args[1]+1);
	}

	else
            return badcommand();
    }

    else if (strcmp(command_args[0], "my_ls") == 0) {
	   DIR *dir = opendir("."); //current dir
	   struct dirent *entry; //directory entry struct
	   char *files[256]; //files array
	   int n = 0;

	   while ((entry = readdir(dir)) != NULL) {//read directory names
		   files [n++] = strdup(entry->d_name);//store names in array
	}
	   closedir(dir);

	   for (int i = 0; i<n-1; i ++){//sort files via ascii 
		   for (int j = i+1; j <n; j ++) {
			   if (strcmp (files[i], files[j]) > 0){
					   char *tmp = files[i];
					   files [i] = files[j];
					   files[j] = tmp;
					   
			   }
		   }

	   }

	   for (int i = 0; i <n; i++){
		   printf("%s\n", files[i]);
	   }

	   return 0;
    }



	   

   else if (strcmp(command_args[0], "my_mkdir") == 0) {
	    if (args_size != 2)
		    return badcommand();
	    if (isalphanumeric(command_args[1])){
	    	int res = mkdir (command_args[1], 0755);
	    }
	    else if (command_args[1][0] == '$' && isalphanumeric(command_args[1]+1)){
		char *name = mem_get_value(command_args[1]+1);

		if (name != NULL && isalphanumeric(name)){
			mkdir(name, 0755);
			return 0;

		}

		else{
		
			printf("Bad command: my_mkdir\n");
			return 0;
		}
			
		}
	    else
		    return badcommand();
	   

	    }
    else if (strcmp(command_args[0], "my_touch") == 0 ){
	    if (args_size != 2)
		    return badcommand();
    	if (isalphanumeric(command_args[1])){
		FILE * file_pointer; //pointer to file
		const char *filename = command_args[1];
		file_pointer = fopen(filename, "w");// assign pointer
		fclose(file_pointer);
		return 0;
	}
	else
		return badcommand();
    
    }

    else if (strcmp(command_args[0], "my_cd") == 0) {
	if (args_size != 2)
		return badcommand();
	if (isalphanumeric(command_args[1])){
    	const char *path = command_args[1];
	if (chdir(path) != 0)
		printf("Bad command: my_cid\n");
		return 0;
	}
    }
    else if (strcmp (command_args[0], "run") == 0) {
    	return run(command_args, args_size);
    }

    else
	    return badcommand();
	
    
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

    while (fgets(line, MAX_USER_INPUT - 1, p) != NULL) {
        errCode = parseInput(line);     // which calls interpreter()
        memset(line, 0, sizeof(line));
    }

    fclose(p);

    return errCode;
}

int run (char *command_arguments[], int num_of_args) {
	pid_t pid = fork();//create new process 
	if (pid == 0){ //child process
		char *args[num_of_args]; //used to store commands after run
					      //
		for (int i=1; i< num_of_args; i++)
			args[i-1] = command_arguments[i];//stored commands

		//make args array end with NULL for execvp
		args[num_of_args-1] = NULL;

		execvp(args[0], args);//replace process with program from command arg
	}
	else if (pid > 0){
		int status;
		wait(&status); //wait returns and status variable contains info about how the child process went
	}

	return 0;
}
