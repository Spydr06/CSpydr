//this is a small C program to auto-increment the build number in a specified header file

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

char* readFile(FILE* fp);

int main(int argc, char* argv[])
{
    if(argc != 2) //check argument count
    {
        printf("Usage: createbuildnumber <target header file>\n");
        exit(1);
    }

    //get the filepath and open it, if the file does not exist, create it
    const char* filePath = argv[1];
    FILE* fp = fopen(filePath, "ab+");
    if(!fp)
    {
        printf("Error opening file \"%s\"\n", filePath);
        exit(1);
    }

    //get the contents of the file
    char* contents = readFile(fp);
    const char* template = "#define BUILD_NUMBER ";
    unsigned long versionNumber = 1;

    //override the file, if it has been used already 
    if(strlen(contents) >= strlen(template))
    {
        //clear the file
        freopen(filePath, "w", fp);

        //get the previous version number
        contents += strlen(template);
        versionNumber = atoi(contents);
        versionNumber++;
    }

    //print the incremented build number to the file
    fprintf(fp, "%s%ld", template, versionNumber);

    //close the filestream
    fclose(fp);
    return 0;
}

char* readFile(FILE* fp)
{
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    char* buffer = (char*) calloc(1, sizeof(char));
    buffer[0] = '\0';

    while((read = getline(&line, &len, fp)) != -1) 
    {
        buffer = (char*) realloc(buffer, (strlen(buffer) + strlen(line) + 1) * sizeof(char));
        strcat(buffer, line);
    }

    if(line) 
    {
        free(line);
    }

    return buffer;
}