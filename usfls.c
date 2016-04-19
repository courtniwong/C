// usfls.c
// Author: Yaoquan Yu
// This program list the files in a directory, and act like linux ls

#include<dirent.h>
#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

// Use insertion sort to sort the list of files
void sort(char **list, int count)
{
    int i, j;
    char *tmp;
    
    for (i = 0; i < count; i++)
    {
        for (j = i + 1; j < count; j++)
        {
            if (strcmp(list[i], list[j]) > 0)
            {
                tmp = list[i];
                list[i] = list[j];
                list[j] = tmp;
            }
        }
    }
}

int main(int argc, char **argv)
{
    DIR *dirp;
    struct dirent *dp;
    int i = 0;
    int all = 0;
    int count = 0;
    char *list[256];
    
    if (argc >= 2)
        if (strcmp(argv[1], "-a") == 0)
            all = 1;
    
    /* Open the current directory */
    dirp = opendir(".");
    if (dirp == NULL) {
        printf("Cannot opendir()\n");
        exit(-1);
    }
    
    while ((dp = readdir(dirp)) != NULL) {
        list[i] = (char *)malloc(sizeof(char)*256);
        strcpy(list[i], dp->d_name);
        i++;
    }
    count = i;
    
    sort(list, count);
    
    for (i = 0; i < count; i++)
        if (strncmp(list[i], ".", 1) != 0 || all)
            printf("%s\n", list[i]);
    closedir(dirp);
    
    return 0;
}