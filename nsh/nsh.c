#define BUFF_SIZE 1024 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>
#include <signal.h>

extern int errno;

/* .... Defines struct ... */
typedef struct {
	char ** colOne;
	char ** colTwo;
} myData;

/* .... Creates struct ... */
myData * newStruct() {
	myData * table = malloc(sizeof(myData*));
	table->colOne = malloc(sizeof(myData)*BUFF_SIZE);
	table->colTwo = malloc(sizeof(myData)*BUFF_SIZE);

	return table;
}

/* .... Adds new $VAR $VAL pair to struct ... */
void addToStruct(myData * table, char * newVar, char * newValue) {
	int i = 0;
	while ((table->colOne[i] != NULL) && (table->colTwo[i] != NULL)) {
		++i;
	}
	table->colOne[i] = malloc(sizeof(myData)*BUFF_SIZE);
	table->colTwo[i] = malloc(sizeof(myData)*BUFF_SIZE);
	strcpy(table->colOne[i], newVar);
	strcpy(table->colTwo[i], newValue);
}

/* .... Changes $VAL for $VAR at location ... */
void changeStruct(myData * table, int location, char * newValue) {
	if ((table->colOne[location] != NULL) && (table->colTwo[location] != NULL)) {
		strcpy(table->colTwo[location], newValue);	
	}
}

/* .... Looks for $VAR in struct, RETURNS 1 if not found, 0 if found and changes struct ... */
int checkStruct(myData * table, char * newVar, char * newValue) {
	int i = 0;
	while ((table->colOne[i] != NULL) && (table->colTwo[i] != NULL)) {
		if ( strcmp(table->colOne[i], newVar) == 0 ) {
			changeStruct(table, i, newValue);
			return 0;
		}
		++i;
	}
	return 1;
}

/* .... Returns $VAL of $VAR ... */
char * getValue(myData * table, char * input) {
	int i = 0;
	while ((table->colOne[i] != NULL) && (table->colTwo[i] != NULL)) {
		if ( strcmp(table->colOne[i], input) == 0 ) {
			strcpy(input, table->colTwo[i]);
			return input;
		}
		++i;
	}
	return input;
}

/* .... Prints struct ... */
void showStruct(myData * table) {
	int i = 0;
	while ((table->colOne[i] != NULL) && (table->colTwo[i] != NULL)) {
		printf("   [%s]=[%s]\n", table->colOne[i], table->colTwo[i]);
		++i;
	}
}

/* .... Frees memory of struct ... */
void killStruct(myData * table) {
	free(table->colOne);
	free(table->colTwo);
	free(table);
}


/* .... Prints current tokens ... */
void showTokens(char ** input) {
	printf("Current tokens are:");
	int i = 0;
	while(input[i] != NULL) {
		printf(" [%s]",input[i]);
		++i;
	}
	printf("\n");
}

/* .... Returns number of tokens ... */
int numTokens(char ** input)
{
	int i = 0;
	while(input[i] != NULL) {
		++i;
	}
	return i;
}


/* ... forks and execs .... */
int execute( char ** input, myData * vars, myData * procs, char * PATH, char ** ARGS, int value) {

	int status = 0;
	char * ENV[] = { 0 };
	pid_t childPID;
	pid_t parentPID;
	
	childPID = fork();
	if (childPID == 0) {
		status = execve(PATH, ARGS, ENV);
		perror("Failed to execute");
		kill(childPID, SIGKILL);
		return(status);
	} else if (childPID < 0) {
		perror("Problem with fork");
		return(status);
	} else {
		if (value == 2) {
			/* add to process list */
			if ( checkStruct(procs, PATH, "ACTIVE") ) {
				addToStruct(procs, PATH, "ACTIVE");
			}
		} else {
			if ( checkStruct(procs, PATH, "INACTIVE") ) {
				addToStruct(procs, PATH, "INACTIVE");
			}
			do {
				parentPID = wait(&status);
			} while ( parentPID != childPID );
		}
		return status;
	}
}


/* .... Parses program control commands ... */
void command(char ** input, myData * vars, myData * procs, int value) {
	
	int i;
	
	/* get num of arguments past cmd */
	size_t numArgs = numTokens(input);
	if (value == 3) {
		--numArgs;
	}
	size_t numCopy = numArgs;
	
	
	/* create the path string */
	char * runPath = malloc(sizeof(char*) * BUFF_SIZE);
	runPath[0] = '\0';
	strcpy(runPath, vars->colTwo[4]);
	
	/* add a backslash if it doesn't exist */
	if ( runPath[strlen(runPath)-1] != '/' ) {
		strcat(runPath, "/");
	}
	
	/* copy or cat cmd based on value and if it has a / or ./ */
	switch(value) {
		case 1:
		case 2:
			if ( input[1][0] != '\0') {
				if ( input[1][0] == '/' ) {
					strcpy(runPath, input[1]);
					break;
				} else if ( input[1][0] == '.' ) {
					if (( input[1][1] != '\0') && ( input[1][1] == '/' )) {
						strcpy(runPath, input[1]);
						break;
					}
				}
			}
			strcat(runPath, input[1]);
			break;
		case 3:
			if ( input[2][0] != '\0') {
				if ( input[2][0] == '/' ) {
					strcpy(runPath, input[2]);
					break;
				} else if ( input[2][0] == '.' ) {
					if (( input[2][1] != '\0') && ( input[2][1] == '/' )) {
						strcpy(runPath, input[2]);
						break;
					}
				}
			}
			strcat(runPath, input[2]);
			break;
	}
	runPath[strlen(runPath)] = 0;
	
	/* .... PRINT PATH ..... 
	printf("runpath is %s\n",runPath);
	*/

	/* create array of strings to hold ARGS */
	char ** runArgs = malloc(sizeof(char**) * numArgs);
	for (i = 0; i < numArgs; ++i) {
		runArgs[i] = malloc(sizeof(char*)*BUFF_SIZE);
		runArgs[i][0] = '\0';
	}
	
	/* copy CMD into first ARG, for convention */
	strcpy(runArgs[0],runPath);
	
	/*copy rest of args */
	numArgs = numTokens(input);
	if ( value == 3 ) {
		for (i = 3; i < numArgs; ++i) {
			strcpy(runArgs[i-2], input[i]);
		}
	} else {
		for (i = 2; i < numArgs; ++i) {
			strcpy(runArgs[i-1], input[i]);
		}
	}
	numArgs = numCopy;
	runArgs[numArgs-1] = 0;
	
	/* .... PRINT ARGS ..... 
	for (i = 0; i < (numArgs-1); ++i) {
		printf("arg [%i] is %s\n",i,runArgs[i]);
	}
	*/
	
	/* attempts to run fork/exec */
	int attempt = execute( input, vars, procs, runPath, runArgs, value);
	if (attempt < 0) {
		perror("Status");
	}
	
	/* free malloc'd memory */
	free(runArgs);
	free(runPath);
}


/* .... Takes input tokens and executes commands ... */
int parser(char ** input, myData * vars, myData * procs) {
	int i;
	char *commands[] = { "done", "do", "back", "tovar", "set", "prompt", "dir", "procs" };
	int size = numTokens(input);
	
	for (i = 0; i < 8; ++i) {
		if (strcmp(input[0], commands[i]) == 0) {
			switch (i) {
				case 0:
					/* ... done ... */
					if (size == 1) {
						printf("Exiting...\n");
						return(0);
					}
					break;
				case 1:
					/* ... do ... */
					if (size >= 2) {
						if ( atoi(vars->colTwo[0]) ) {
							showTokens(input);
						}
						command(input, vars, procs, 1);
						return(1);
					}
					break;
				case 2:
					/* ... back ... */
					if (size >= 2) {
						if ( atoi(vars->colTwo[0]) ) {
							showTokens(input);
						}
						command(input, vars, procs, 2);
						return(2);
					}
					break;
				case 3:
					/* ... tovar ... */
					if (size >= 3) {
						if ( atoi(vars->colTwo[0]) ) {
							showTokens(input);
						}
						if (checkStruct(vars, input[1],"")) {
								addToStruct(vars, input[1],"");
							}
						command(input, vars, procs, 3);
						return(3);
					}
					break;
				case 4:
					/* ... set ... */
					if (size == 3) {
						if (isalpha(input[1][0])) {
							if (checkStruct(vars, input[1],input[2])) {
								addToStruct(vars, input[1],input[2]);
							}
							if ( atoi(vars->colTwo[0]) ) {
								showTokens(input);
							}
							return(4);
						}
					}
					break;					
				case 5:
					/* ... prompt ... */
					if (size == 2) {
						if ( atoi(vars->colTwo[0]) ) {
							showTokens(input);
						}
						changeStruct(vars, 2, input[1]);
						return(5);
					}
					break;
				case 6:
					/* ... dir ... */
					if (size == 2) {						
						if ( atoi(vars->colTwo[0]) ) {
							showTokens(input);
						}
						char * dirName = input[1];						
						if ( chdir(dirName) != 0 ) {
							errno = ENOTDIR;
							perror("Could not change DIR");
						}
						vars->colTwo[3] = getcwd(vars->colTwo[3],BUFF_SIZE);
						return(6);
					}
					break;
				case 7:
					/* ... procs ... */
					if (size == 1) {
						if ( atoi(vars->colTwo[0]) ) {
							showTokens(input);
						}
						printf("Processes:\n");
						showStruct(procs);
						return(7);
					}
					break;
			}
		}
	}
	if ( atoi(vars->colTwo[0]) ) {
		showTokens(input);
	}
	errno = EIO;
	perror("Could not parse input");
	printf("Try again\n");
	return(8);
}


/* .... Looks for $VAR in string and replaces it with $VAL ... */
void findVar(myData * vars, char * input, int n) {
	
	int i;
	size_t size = strlen(input);
	
	char * first = malloc(sizeof(char*)*BUFF_SIZE);
	first[0] = '\0';
	char * second = malloc(sizeof(char*)*BUFF_SIZE);
	second[0] = '\0';
	char * third = malloc(sizeof(char*)*BUFF_SIZE);
	third[0] = '\0';
	
	for (i = 0; i < n; ++i) {
		first[i] = input[i];
	}
	first[i] = '\0';
	
	n++;
	for (i = n; i < size; ++i) {
		if ( (input[i] == '\0') || (input[i] == '$') || (input[i] == ' ') || (input[i] == '"') ) {
			break;
		}
		second[i-n] = input[i];
	}
	second[(i-n)] = '\0';
	
	n = i;
	for (i = n; i < size; ++i) {
		third[i-n] = input[i];
	}
	third[(i-n)] = '\0';
	
	/* check $VAR if defined, replace with definition */
	strcpy(second, getValue(vars,second));
	
	/* combine strings */
	strcat(first, second);
	strcat(first, third);
	
	strcpy(input, first);
		
	/* free malloc'd memory */
	free(third);
	free(second);
	free(first);
}


/* .... Takes string from input and turns into tokens ... */
void scanner(char * input, char ** output, myData * vars) {
	int i = 0;
	int j = 0;
	int stringFlag = 0;

	char * buffer = malloc(sizeof(char*)*BUFF_SIZE);
	buffer[0] = '\0';
	char * stringCopy = malloc(sizeof(char*)*BUFF_SIZE);
	stringCopy[0] = '\0';
	
	/* remove comment if it exists */
	for (i = 0; i < strlen(input); ++i) {
		if (input[i] == '%') {
			input[i] = '\0';
			break;
		}
	}
	
	/* remove STRING if it exists to add as last token later */
	for (i = 0; i < strlen(input); ++i) {
		if (input[i] == '"') {
			stringFlag = 1;
			input[i] = ' ';
			int m = 0;
			int n = i + 1;
			while (input[n] != '\0') {
				if (input[n] == '"') {
					input[n] = ' ';
					break;
				}
				stringCopy[m] = input[n];
				input[n] = ' ';
				++m;
				++n;
			}
			stringCopy[m] = '\0';
			break;
		}
	}
	
	/* tokenize */
	i = 0;
	buffer = strtok(input, " \n");
	while(buffer != NULL) {
		strcpy(output[i], buffer);
		buffer = strtok( NULL, " \n");
		++i;
	}
	if (stringFlag) {
		strcpy(output[i], stringCopy);
		++i;
	}
	output[i] = NULL;
	
	/* check for $VAR in each token*/
	i = 0;
	while(output[i] != NULL) {			
		for (j = 0; j < strlen(output[i]); ++j) {
			if (output[i][j] == '$') {
				findVar(vars, output[i], j);
			}
		}
		++i;
	}
	
	/* free malloc'd */
	free(stringCopy);
	free(buffer);
}


/* .... Reads input into a string ... */
void readLine(char * input, myData * vars) {
	printf("%s",vars->colTwo[2]);  // COMMAND PROMPT
	fgets(input, BUFF_SIZE, stdin);
	int size = strlen(input);
	input[size-1] = '\0';
}



int main(int argc, char *argv[]) {
	
	int i;
	int action = 0;
	
	char commandLine[BUFF_SIZE];
	char ** scannedLine = malloc(sizeof(char**) * BUFF_SIZE);
	for (i = 0; i < BUFF_SIZE; ++i) {
		scannedLine[i] = malloc(sizeof(char*) * BUFF_SIZE);
		scannedLine[i][0] = '\0';
	}
	
	char cwd[BUFF_SIZE];
	strcpy(cwd, getcwd(cwd,BUFF_SIZE));
	
	myData * vars = newStruct();
	addToStruct(vars, "ShowTokens", "1");		// vars[0]
	addToStruct(vars, "ShowVars", "1");		// vars[1]
	addToStruct(vars, "PROMPT", "nsh > ");		// vars[2]
	addToStruct(vars, "CWD", cwd);			// vars[3]
	addToStruct(vars, "PATH", "/bin");		// vars[4]

	myData * procs = newStruct();
	addToStruct(procs, "NAME", "STATUS");		// procs[0]
	
	do {
		if (atoi(vars->colTwo[1])) {
			printf("Variables:\n");
			showStruct(vars);
		}
		readLine(commandLine, vars);
		scanner(commandLine, scannedLine, vars);
		action = parser(scannedLine, vars, procs);
		for (i = 0; i < BUFF_SIZE; ++i) {
			scannedLine[i] = malloc(sizeof(char*) * BUFF_SIZE);
			scannedLine[i][0] = '\0';
		}
	} while (action);
	
	/* FREE MALLOC'D MEMORY */	
	killStruct(vars);
	killStruct(procs);
	free(scannedLine);
		
	return(0);	
}
