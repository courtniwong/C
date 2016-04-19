// usfsh.c
// Author: Yaoquan Yu
// This program works like linux shell.

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdbool.h>
#include <string.h>

#define SIZE 256

char *history_list[256];
int history_count;

void print_commands(char **array, int num_arguments)
{
    int i;
    for (i = 0; i < num_arguments; i++)
        printf("%s\n", array[i]);
}

bool read_line(int fd_in, char *input)// implement tab
{
    char buf[1];
    int count = 0;
    int i = 0;
    
    while ((count = read(fd_in, buf, 1)) > 0)
    {
        input[i] = buf[0];
        i += 1;
        if (buf[0] == '\n')
            break;
    }
    if (count <= 0)
        return false;
    else
        return true;
}

void print(char *input)
{
    write(1, input, strlen(input));
}

void local_execution(char **array)
{
    pid_t id;
    id = fork();
    
    if (id == 0)
    {
        if (execvp(array[0], array) < 0)
            printf("local exectution fail\n");
    }
    
    id = wait(NULL);
}

void change_dir(char **array)
{
    chdir(array[1]);
}

// get execution type of a program, pipe, file redirection, or base linux program
void get_exec_type(char **array, int num_arguments, char **argv1, char **argv2, int *type)
{
    int i, j;
    j = 0;
    for (i = 0; i < num_arguments; i++)
    {
        if (strcmp(array[i], "|") == 0)
        {
            *type = 1;
            i++;
            break;
        }
        else if (strcmp(array[i], ">") == 0)
        {
            *type = 2;
            i++;
            break;
        }
        else if (strcmp(array[i], "<") == 0)
        {
            *type = 3;
            i++;
            break;
        }
        else
        {
            argv1[j] = (char *) malloc(sizeof(char)*SIZE);
            argv1[j] = array[i];
            j++;
            argv1[j] = NULL;
        }
    }
    
    j = 0;
    while (i < num_arguments)
    {
        argv2[j] = (char *) malloc(sizeof(char)*SIZE);
        argv2[j] = array[i];
        i++;
        j++;
        argv2[j] = NULL;
    }
}

// execute programs base on the type, name, and arguments
void general_execution(int start, char **array, int num_arguments)
{
    char *local_array[20];
    int i = 0;
    int type = 0;
    char *argv1[20];
    char *argv2[20];
    
    get_exec_type(array, num_arguments, argv1, argv2, &type);
    
    if (type == 0)
    {
        pid_t id;
        id = fork();
        
        if (id == 0)
        {
            if (execvp(argv1[0], argv1) < 0)
                printf("general executation 1 fail\n");
        }
        id = wait(NULL);
    }
    else if (type == 1)
    {
        pid_t id;
        int fildes[2];
        pipe(fildes);
        id = fork();
        
        if (id == 0)
        {
            close(0);
            dup(fildes[0]);
            close(fildes[1]);
            if (execvp(argv2[0], argv2) < 0)
                printf("Pipe fail child\n");
        }
        else
        {
            close(fildes[0]);
            close(1);
            dup(fildes[1]);
            if (execvp(argv1[0], argv1) < 0)
                printf("Pipe fail parent\n");
            close(fildes[1]);
            id = wait(NULL);
        }
    }
    else if (type == 2)
    {
        pid_t id;
        id = fork();
        if (id == 0)
        {
            int fd_out;
            fd_out = open(argv2[0], O_CREAT | O_WRONLY, 0644);
            close(1);
            dup(fd_out);
            if (execvp(argv1[0], argv1) < 0)
                print("general executation > fail\n");
        }
        id = wait(NULL);
    }
    else if (type == 3)
    {
        pid_t id;
        id = fork();
        if (id == 0)
        {
            int fd_in;
            fd_in = open(argv2[0], O_RDONLY);
            close(0);
            dup(fd_in);
            if (execvp(argv1[0], argv1) < 0)
                print("general executation > fail\n");
        }
        id = wait(NULL);
    }
    else
        printf("Fail general execution type\n");
}

// print the user's command history
void history()
{
    int i;
    char *tmp;
    
    for (i = 1; i < history_count; i++)
    {
        tmp = (char *) malloc(sizeof(char)*256);
        sprintf(tmp, "  %d:  %s", i, history_list[i-1]);
        print(tmp);
    }
}

void execute_command(char **array, int *done, int num_arguments)
{
    if (strcmp(array[0], "exit") == 0)
        *done = 1;
    else if (strncmp(array[0], "./", 2) == 0)// compare the first 2 chars
        local_execution(array);
    else if (strcmp(array[0], "cd") == 0)
        change_dir(array);
    else if (strcmp(array[0], "history") == 0)
        history();
    else
        general_execution(0, array, num_arguments);
}

void parse_command(char *command, char **array, int *num_arguments)
{
    char line[20];
    int count = 0;
    int i, j;
    j = 0;
    for (i = 0; i < strlen(command); i++)
    {
        if (command[i] != '\n' && command[i] != ' ')
        {
            line[count] = command[i];
            count++;
        }
        else
        {
            if (count > 0)
            {
                line[count] = '\0';
                array[j] = (char *)malloc(sizeof(char)*256);
                strcpy(array[j], line);
                j++;
                count = 0;
            }
        }
    }
    *num_arguments = j;
    array[j] = NULL;
}

void get_dir(char *dir)
{
    char tmp[256];
    getcwd(tmp, sizeof(tmp));
    strcpy(dir, tmp);
}

void get_command(char **command)
{
    char *dir = malloc(sizeof(char)*256);
    get_dir(dir);
    
    print("[");
    print(dir);
    print(" ]$ ");
    
    *command = (char *) malloc(sizeof(char)*SIZE);
    history_list[history_count] = (char *) malloc(sizeof(char)*SIZE);
    read_line(0, *command);
    strcpy(history_list[history_count], *command);
    history_count++;
}

int main(int argc, char **argv)
{
    int done = 0;
    int num_arguments = 0;
    int i = 0;
    char *command;
    char *array[10];
    
    history_count = 0;
    
    while (!done)
    {
        get_command(&command);
        parse_command(command, array, &num_arguments);
        execute_command(array, &done, num_arguments);
    }
    
    return 0;
}

