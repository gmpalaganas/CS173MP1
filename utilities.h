#include <string.h>

#define TRUE 1
#define FALSE 0
#define DELIMITERS_WHITESPACE " \n\r"
#define DELIMITERS_PERIOD "."

typedef int boolean;

/* Utility Functions */
char* getInput(char* inputStorage) {
    inputStorage = (char*) malloc(sizeof(char));
    char nextChar = '\n';
    int ctr;
    for (ctr = 0; (nextChar = getchar()) != '\n' && nextChar != EOF; ctr++) {
        inputStorage = (char*)realloc(inputStorage, (ctr + 1)*sizeof(char));
        inputStorage[ctr] = nextChar;
    }
    inputStorage[ctr] = '\0';
    return inputStorage;
}

boolean isAlphaNumeric(char* input) {
    if (input == NULL) return FALSE;

    int len = strlen(input);
    if (len <= 0) return FALSE;

    int i;
    for (i = 0; i < len; i++) {
        if (!isalnum(input[i])) return FALSE;
    }
    return TRUE;
}

//Gets the file name from a command like "UPLOAD <filename>"
char* getFileName(char* command, char* filenameStorage) {
	filenameStorage = (char*)malloc(sizeof(char)*(strlen(command) + 1));
    strcpy(filenameStorage, command);
    strtok(filenameStorage, DELIMITERS_WHITESPACE);
    return strtok(NULL, DELIMITERS_WHITESPACE);
}

boolean isFilenameValid(char* input) {
	char* inputCopy = (char*)malloc(sizeof(char)*(strlen(input)+1));
	strcpy(inputCopy, input);
	boolean result = TRUE;
	if (!isAlphaNumeric(strtok(inputCopy, ".")) && isAlphaNumeric(strtok(NULL, DELIMITERS_WHITESPACE))) result = FALSE;
	free(inputCopy);
    return result;
}

int getFileSize(FILE* file){
    int ret = 0;
    fseek(file,0,SEEK_END);
    ret = ftell(file); 
    //fseek(file,0,SEEK_SET);
    rewind(file);

    return ret;
}
