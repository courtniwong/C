// usfsort.c
// Author: Yaoquan Yu
// This program takes lines of strings from user, sort all lines
// and print the sorted list or write the sorted list to file
// Compile: gcc -o usfsort usfsort.c
// Useage: ./usfsort input_file output_file -r
// All arguments are optional.
// Special case: if only one file provided, the program assume that
//               is an input file.

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#define SIZE 256

struct node_s
{
    char *data;
    struct node_s *next_p;
};

struct list_s
{
    struct node_s *head_p, *tail_p;
};

int open_input_file(char *filename)
{
    int fd_in;
    
    fd_in = open(filename, O_RDONLY);
    if (fd_in < 0)
    {
        printf("Cannot open %s\n", filename);
        exit(-1);
    }
    
    return fd_in;
}

int open_output_file(char *filename)
{
    return open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
}

// read a line from file or user
bool read_line(int fd_in, struct node_s *tmp)
{
    char buf[1];
    char *line = malloc(sizeof(char)*SIZE);
    int count = 0;
    int i = 0;
    
    while ((count = read(fd_in, buf, 1)) > 0)
    {
        line[i] = buf[0];
        i += 1;
        if (buf[0] == '\n')
            break;
    }
    if (count == 0)
        line[i++] = '\n';
    line[i] = '\0';
    tmp->data = line;
    tmp->next_p = NULL;
    if (count <= 0)
        return false;
    else
        return true;
}

void print_list(struct list_s *list)
{
    struct node_s *curr_node;
    curr_node = list->head_p;
    
    while (curr_node != NULL)
    {
        printf("%s", curr_node->data);
        curr_node = curr_node->next_p;
    }
    printf("finish print_list\n");
}

// insert an line string to the list
void insert_to_list(struct list_s *list, struct node_s *tmp)
{
    if (list->head_p == NULL)
    {
        tmp->next_p = NULL;
        list->head_p = tmp;
        list->tail_p = tmp;
    }
    else
    {
        tmp->next_p = NULL;
        list->tail_p->next_p = tmp;
        list->tail_p = tmp;
    }
}

// read all inputs from file or user
void read_input(int fd_in, struct list_s *list)
{
    bool more = true;
    struct node_s *tmp;
    
    while (more)
    {
        tmp = (struct node_s *) malloc(sizeof(struct node_s));
        more = read_line(fd_in, tmp);
        if (tmp->data[0] != '\n')
            insert_to_list(list, tmp);
    }
}

// Use insertion sort to sort the list of lines
void sort(struct list_s *list, int reverse)
{
    struct node_s *i, *j;
    char *tmp;
    
    i = list->head_p;
    j = i->next_p;
    
    while (i->next_p != NULL)
    {
        while (j != NULL)
        {
            if (strcmp(i->data, j->data) * reverse > 0)
            {
                tmp = i->data;
                i->data = j->data;
                j->data = tmp;
            }
            j = j->next_p;
        }
        i = i->next_p;
        j = i->next_p;
    }
}

void write_to_file(int fd_out, struct list_s *list)
{
    struct node_s *tmp = list->head_p;
    
    while (tmp != NULL)
    {
        write(fd_out, tmp->data, strlen(tmp->data));
        tmp = tmp->next_p;
    }
}

void read_from_user(struct list_s *list)
{
    int read_from_stdin = 0;
    read_input(read_from_stdin, list);
}

void no_input_file_and_output_file(struct list_s *list, int reverse)
{
    read_from_user(list);
    printf("\n\n");
    sort(list, reverse);
    printf("sorted lists: \n");
    print_list(list);
}

void no_output_file(char *input_file, struct list_s *list, int reverse)
{
    int fd_in;
    fd_in = open_input_file(input_file);
    read_input(fd_in, list);
    sort(list, reverse);
    print_list(list);
    close(fd_in);
}

// when both input file and output file are provided.
void general_case(char *input_file, char *output_file, struct list_s *list, int reverse)
{
    int fd_in, fd_out;
    fd_in = open_input_file(input_file);
    fd_out = open_output_file(output_file);
    read_input(fd_in, list);
    sort(list, reverse);
    write_to_file(fd_out, list);
    close(fd_in);
    close(fd_out);
}

int main(int argc, char *argv[])
{
    int fd_in, fd_out, num_args;
    int reverse = 1;
    num_args = argc;
    
    // setup the list
    struct list_s *list = (struct list_s *) malloc(sizeof(struct list_s));
    list->head_p = NULL;
    list->tail_p = NULL;
    
    // handle the case when the program needs to sort in reverse order
    if (strcmp(argv[argc - 1], "-r") == 0)
    {
        reverse = -1;
        num_args -= 1;
    }
    
    // both input and output file are not provided
    if (num_args == 1)
        no_input_file_and_output_file(list, reverse);
    
    // only input file provided
    if (num_args == 2)
        no_output_file(argv[1], list, reverse);
    
    // both input and output file are provided
    if (num_args == 3)
        general_case(argv[1], argv[2], list, reverse);
    
    // report error when there are too many arguments
    if (num_args < 1 || num_args > 3)
    {
        printf("Too many arguments\n");
        printf("Usage: ./usfsort input_file output_file -r\n");
        printf("All arguments are optional\n");
    }
    
    return 0;
}

