#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#define TRUE 1
#define FALSE 0
#define DELIMITERS_WHITESPACE " \n\r"
#define DELIMITERS_PERIOD "."
#define KILOBYTE 1024

typedef int boolean;
typedef struct dirent directory_entity;

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
    rewind(file);

    return ret;
}

void writeDirToFile(FILE* file, char* dir_name, boolean giveSize){

    DIR *dir;
    dir = opendir(dir_name);
    directory_entity *entity;
	int count = 0;
	
    if(dir == NULL)
        printf("Error Opening Directory");
    else{
        while((entity = readdir(dir)) != NULL){
            //Do not include hidden files
            if(entity->d_name[0] != '.'){
                if(giveSize == TRUE){
                    FILE* curFile = fopen(entity->d_name,"r");
                    int size = getFileSize(curFile);
                    if(size < KILOBYTE)
                        fprintf(file,"%s -- %d bytes\n", entity->d_name,size);
                    else{
                        float f_size = (float)size / (float)KILOBYTE;
                        fprintf(file,"%s -- %0.1f kB\n", entity->d_name,f_size);
                    }
                    fclose(curFile);
                }else
                    fprintf(file,"%s\n", entity->d_name);
                    
                count++;
            }
        }
        
        if(count == 0)
        	fprintf(file,"NO FILES IN DIRECTORY");
        
        closedir(dir);
    }
}


void printFileContents(FILE* file){
    
    char c;
    while((c = getc(file)) != EOF)
        putchar(c);

}
