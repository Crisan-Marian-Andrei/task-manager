#include <linux/limits.h>
#include <limits.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/select.h>

typedef struct
{
    int PID;
    char* name;
    char state;
    unsigned long utime;
    unsigned long stime;
    double cpu_usage;
    int visited;
    double memory;
}ProcessInfo;

struct nod
{
    ProcessInfo* info;
    struct nod* left;
    struct nod* right;
    int height;
};

typedef struct nod Node;

void printProccess();
Node* populateProcessTree(Node* root);
int printTable(Node* root, char sort_mode, int* selected_row, int reverse_sort, char* search_filter, int is_searching);
void freeTree(Node* root);
void enableRawMode();
void disableRawMode();