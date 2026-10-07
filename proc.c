#include "functions.h"
#include <signal.h>

int main()
{
    enableRawMode(); 
    printf("\033[?1049h");
    printf("\033[?25l");

    Node* root = NULL;
    int running = 1;
    char sort_mode = 'c'; 
    int selected_row = 0;
    int reverse_sort = 0;

    char search_filter[256] = "";
    int is_searching = 0;

    while(running)
    {
        printf("\033[H"); 
        
        root = populateProcessTree(root);
        int selected_pid = printTable(root, sort_mode, &selected_row, reverse_sort, search_filter, is_searching);
        
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);

        struct timeval timeout;
        timeout.tv_sec = 1; 
        timeout.tv_usec = 0;

        if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout) > 0)
        {
            char c;
            read(STDIN_FILENO, &c, 1);

            if (is_searching)
            {
                if (c == '\n' || c == '\r')
                {
                    is_searching = 0;
                }
                else if (c == 127 || c == '\b')
                {
                    int len = strlen(search_filter);
                    if (len > 0) search_filter[len - 1] = '\0';
                    selected_row = 0;
                }
                else if (c == '\033')
                {
                    is_searching = 0;
                }
                else if (c >= 32 && c <= 126 && strlen(search_filter) < 255)
                {
                    int len = strlen(search_filter);
                    search_filter[len] = c;
                    search_filter[len + 1] = '\0';
                    selected_row = 0;
                }
            }
            else
            {
                if (c == 'q') running = 0;
                else if (c == 'm') sort_mode = 'm';
                else if (c == 'c') sort_mode = 'c';
                else if (c == 'r') reverse_sort = !reverse_sort;
                else if (c == '/') is_searching = 1;
                else if (c == 'k' && selected_pid != -1)
                {
                    kill(selected_pid, SIGKILL);
                }
                else if (c == '\033')
                {
                    char seq[3];
                    if (read(STDIN_FILENO, &seq[0], 1) == 0) continue;
                    if (read(STDIN_FILENO, &seq[1], 1) == 0) continue;
                    if (seq[0] == '[')
                    {
                        if (seq[1] == 'A') selected_row--;
                        if (seq[1] == 'B') selected_row++;
                    }
                }
            }
        }
    }
    freeTree(root);
    disableRawMode();
    printf("\033[?1049l");
    printf("\033[?25h");
    return 0;
}