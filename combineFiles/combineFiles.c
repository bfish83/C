#define BUFF_SIZE 128

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

int validNum(char * string) { /* validates if string contains only ints*/
    int i = 0;
    while ( i < strlen(string) ) {
           if( !isdigit(*(string + i)) )
           return 0;
           ++i;
    }
    return 1;
}

long getNum(char * string) { /*converts string to int and returns it -or- returns 0*/
	long N;
	char * numAsString;
	if (strlen(string) > 2) {
		numAsString = string + 2;
		if (validNum(numAsString)) {
			N = atoi(numAsString);
		} else {
			N = 0;
		}
	} else {
		N = 0;
	}
	return N;
}

int fileExists(char * string) { /*checks to see whether file can be read*/
	FILE *src;
	if ((src = fopen(string, "rb"))) {
		fclose(src);
		return 1;
	} else {
		return 0;
	}
}
	
int main(int argc, char *argv[]) { /* argc number of arguments, argv array of strings */
	int i;
	long N = 0;
	char option;
	FILE *src;
	int goodOption;
	long size;
	char *buffer;
	long start;
	char newBuff[BUFF_SIZE];
	size_t bufferSize = 1;

	for (i = 1; i < argc; ++i) {
		
		if (i % 2) { /*if odd, parse options */
			if ((strlen(argv[i]) >= 2) && (argv[i][0] == '-')) {
				if ((strlen(argv[i]) == 2) && (argv[i][1] == 'a')) {
					option = argv[i][1];
					N = 0;
					goodOption = 1;						
				} else if ((argv[i][1] == 's') || (argv[i][1] == 'e')) {
					option = argv[i][1];
					N = getNum(argv[i]);
					goodOption = N;
				} else {
					goodOption = 0;
				}
			} else {
				goodOption = 0;
			}
			
		} else { /* if even, parse filename */
			if (goodOption) {
				
				if (argv[i][0] == '-') { /*if file is of type '-', read from stdin*/
					
					int flag = 1;
					buffer = malloc(sizeof(char) * BUFF_SIZE);
					buffer[0] = '\n';
					
					while ((fgets(newBuff, BUFF_SIZE, stdin)) != NULL) {
						bufferSize += strlen(newBuff);
						buffer = realloc(buffer, bufferSize);
						if (buffer == NULL) {
							errno = 5;
							perror("");
							flag = 0;
							break;
						}
						strcat(buffer, newBuff);
					}
					if (ferror(stdin)) {
						free(buffer);
						errno = 5;
						perror("");
						flag = 0;
					}
					if (flag) {
						printf("%s\n", buffer);
					}
					free(buffer);
					
				} else if (fileExists(argv[i])) { /*if file is standard file, read file and output using options*/
					
					src = fopen(argv[i], "rb");
					fseek(src, 0, SEEK_END);
					size = ftell(src);
					rewind(src); 
					if (N > size) {
						N = size;
					}
					switch (option) {
						case 'a':
							N = size;
						case 's':
							start = 0;
							break;
						case 'e':
							start = (size - N);
							break;
					}
					buffer = (char *)malloc(N);
					fseek(src, start, SEEK_SET);
					fread(buffer, N, 1, src);
					printf("%s\n", buffer);
					fclose(src);
					free(buffer);
					
				} else { /* error if can't access file */ 						errno = 2;
					perror(argv[i]);
				}
				
			} else { /*error if given invalid SPEC*/
				errno = 22;
				perror(argv[i-1]);
			}
		}
	}
	
	return(0);
}
