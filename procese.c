#include "functions.h"

struct termios orig_termios;

int countNodes(Node* root)
{
    if ( root == NULL ) return 0;
    return 1 + countNodes( root-> left ) + countNodes( root->right );
}

void disableRawMode() 
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() 
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode); // Se asigură că terminalul revine la normal la ieșire

    struct termios raw = orig_termios;
    // Dezactivăm ECHO (afișarea pe ecran) și ICANON (modul care așteaptă Enter)
    raw.c_lflag &= ~(ECHO | ICANON);
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void printProccess()
{
    DIR *d;
    struct dirent * dir;
    char fullpath[PATH_MAX + 1];
    char temp_path[PATH_MAX + 1];
    d = opendir("/proc/");
    if(d != NULL)
    {
       while((dir = readdir(d)) != NULL)
       {
         snprintf(temp_path, sizeof(temp_path), "/proc/%s", dir->d_name);
         realpath(temp_path, fullpath); 
         printf("[%s]\n", fullpath);
         printf("%s\n", dir->d_name);
       }
       closedir(d);
    }
    return;
}

int max(int a, int b)
{
    return ((a > b)? a:b);
}

int nodeHeight(Node* root)
{
    if (root == NULL)
    {
        return -1;
    }
    else
    {
        return root->height;
    }
}

Node* rightRotation(Node* z)
{
    Node* y = z->left;
    Node* T3 = y->right;
    //rotate
    y->right = z;
    z->left = T3;
    //update the heights
    z->height = max(nodeHeight(z->left), nodeHeight(z->right)) + 1;
    y->height = max(nodeHeight(y->left), nodeHeight(y->right)) + 1;
    return y;
}

Node* leftRotation(Node* z)
{
    Node* y = z->right;
    Node* T2 = y->left;
    //rotate
    y->left = z;
    z->right = T2;
    //update the heights
    z->height = max(nodeHeight(z->left), nodeHeight(z->right)) + 1;
    y->height = max(nodeHeight(y->left), nodeHeight(y->right)) + 1;
    return y;
}

Node* LRRotation(Node* z)
{
    z->left = leftRotation(z->left);
    return rightRotation(z);
}

Node* RLRotation(Node* z)
{
    z->right = rightRotation(z->right);
    return leftRotation(z);
}

Node* insertNode(Node* node, int PID, char state, char* name, unsigned long utime, unsigned long stime, double memory)
{
    //insert node
    if (node == NULL)
    {
        node = (Node*)malloc(sizeof(Node));
        node->info = malloc(sizeof(*node->info));
        node->info->PID = PID;
        node->info->name = strdup(name);
        node->info->state = state;
        node->info->cpu_usage = 0;
        node->info->memory = memory;
        node->info->stime = stime;
        node->info->utime = utime;
        node->height = 0;
        node->info->visited = 1;
        node->left = node->right = NULL;
        return node;
    }

    if (PID < node->info->PID)
    {
        node->left = insertNode(node->left, PID, state, name, utime, stime, memory);
    }
    else if (PID > node->info->PID)
    {
        node->right = insertNode(node->right, PID, state, name, utime, stime, memory);
    }
    else return node;

    //update the heights

    node->height = 1 + max(nodeHeight(node->left), nodeHeight(node->right));

    //balance factor

    int k = (nodeHeight(node->left) - nodeHeight(node->right));

    //balance the tree

    //LL case
    if (k > 1 && PID < node->left->info->PID)
    {
        return rightRotation(node);
    }

    //RR case
    if (k < -1 && PID > node->right->info->PID)
    {
        return leftRotation(node);
    }

    //LR
    if (k > 1 && PID > node->left->info->PID)
    {
        return LRRotation(node);
    }

    //RL
    if (k < -1 && PID < node->right->info->PID)
    {
        return RLRotation(node);
    }

    return node;
}

Node* search(Node* root, int PID)
{
    if (root == NULL || root->info->PID == PID)
    {
        return root;
    }
    if (root->info->PID < PID)
    {
        return search(root->right, PID);
    }
    return search(root->left, PID);
}

void findDeadProcesses(Node* root, int* dead_pids, int* count)
{
    if (root == NULL)
        return;
    findDeadProcesses(root->left, dead_pids, count);

    if (root->info->visited == 0) 
    {
        dead_pids[*count] = root->info->PID;
        (*count)++;
    } 
    else 
    {
        root->info->visited = 0; 
    }

    findDeadProcesses(root->right, dead_pids, count);
}

Node* minValueNode(Node* node)
{
    Node* current = node;
    while (current->left != NULL)
        current = current->left;
    return current;
}

Node* deleteNode(Node* root, int PID)
{
    if (root == NULL)
        return root;

    if (PID < root->info->PID)
        root->left = deleteNode(root->left, PID);
    else if (PID > root->info->PID)
        root->right = deleteNode(root->right, PID);
    else
    {
        
        // one child or none
        if ((root->left == NULL) || (root->right == NULL))
        {
            Node* temp = root->left ? root->left : root->right;

            if (temp == NULL) // none
            {
                free(root->info->name);
                free(root->info);
                free(root);
                root = NULL;
            }
            else // one
            {
                Node* old_root = root;
                root = temp;
                free(old_root->info->name);
                free(old_root->info);
                free(old_root);
            }
        }
        else
        {
            // 2 children
            Node* temp = minValueNode(root->right);
            root->info->PID = temp->info->PID;
            free(root->info->name);
            root->info->name = strdup(temp->info->name);
            root->info->state = temp->info->state;
            root->info->utime = temp->info->utime;
            root->info->stime = temp->info->stime;
            root->info->cpu_usage = temp->info->cpu_usage;
            root->right = deleteNode(root->right, temp->info->PID);
        }
    }
    if (root == NULL)
        return root;
    root->height = 1 + max(nodeHeight(root->left), nodeHeight(root->right));

    // balance factor
    int k = nodeHeight(root->left) - nodeHeight(root->right);
    
    int balanceLeft = nodeHeight(root->left ? root->left->left : NULL) - nodeHeight(root->left ? root->left->right : NULL);
    int balanceRight = nodeHeight(root->right ? root->right->left : NULL) - nodeHeight(root->right ? root->right->right : NULL);

    // LL Case
    if (k > 1 && balanceLeft >= 0)
        return rightRotation(root);

    // LR Case
    if (k > 1 && balanceLeft < 0)
        return LRRotation(root);

    // RR Case
    if (k < -1 && balanceRight <= 0)
        return leftRotation(root);

    // RL Case
    if (k < -1 && balanceRight > 0)
        return RLRotation(root);

    return root;
}

Node* populateProcessTree(Node* root)
{
    DIR *path;
    struct dirent *proc;
    char fullpath[PATH_MAX + 1];
    char temp_path[PATH_MAX + 1];
    path = opendir("/proc/");
    
    if(path != NULL)
    {
        while((proc = readdir(path)) != NULL)
        {
            if (!(proc->d_name[0] >= '0' && proc->d_name[0] <= '9'))
                continue;
            
            snprintf(temp_path, sizeof(temp_path), "/proc/%s/stat", proc->d_name);
            FILE* f = fopen(temp_path, "r");
            if (f == NULL) continue;

            char buffer[2048];
            if (fgets(buffer, sizeof(buffer), f) == NULL) {
                fclose(f);
                continue;
            }
            fclose(f);

            int pid = atoi(proc->d_name);
            char name[256] = "unknown";
            char state = 'S';
            unsigned long utime = 0, stime = 0;

            // Căutăm manual parantezele pentru a ocoli procesele cu nume ciudate (ex: "(kworker/1:0 )")
            char* start = strchr(buffer, '(');
            char* end = strrchr(buffer, ')');
            
            if (start && end && end > start) {
                int len = end - start - 1;
                if (len >= sizeof(name)) len = sizeof(name) - 1;
                strncpy(name, start + 1, len);
                name[len] = '\0';

                // Sărim peste paranteza închisă și spațiu, și citim cu siguranță absolută timpul
                char* p = end + 2; 
                sscanf(p, "%c %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %lu %lu", 
                       &state, &utime, &stime);
            }

            // Citire memorie sigură din statm
            unsigned long rss_pages = 0;
            snprintf(temp_path, sizeof(temp_path), "/proc/%s/statm", proc->d_name);
            FILE* f_statm = fopen(temp_path, "r");
            if (f_statm != NULL) {
                unsigned long total_vms = 0;
                if (fscanf(f_statm, "%lu %lu", &total_vms, &rss_pages) != 2) {
                    rss_pages = 0;
                }
                fclose(f_statm);
            }

            unsigned long page_size = sysconf(_SC_PAGESIZE);
            double memory_mb = (rss_pages * page_size) / 1024.0 / 1024.0;

            Node* existing = search(root, pid);
            if (existing == NULL)
            {
                // Inserăm nodul (presupunând că ai modificat insertNode să accepte parametrul memory_mb)
                // Dacă primești eroare de compilare aici, șterge ", memory_mb" de la final.
                root = insertNode(root, pid, state, name, utime, stime, memory_mb);
            }
            else
            {
                // Calculul ORIGINAL de CPU care funcționa 
                unsigned long total_ticks_trecute = (utime + stime) - (existing->info->utime + existing->info->stime);
                int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
                long int clock_ticks = sysconf(_SC_CLK_TCK);
                
                double secunde_consumate = (double)total_ticks_trecute / clock_ticks;
                existing->info->cpu_usage = (secunde_consumate * 100.0) / num_cores;

                // Actualizăm instanța pentru a putea face diferența de timp la următoarea trecere
                existing->info->utime = utime; 
                existing->info->stime = stime;
                existing->info->state = state;
                existing->info->memory = memory_mb;
                existing->info->visited = 1;
            }
        }
        closedir(path);

        int total_nodes = countNodes(root);
        if (total_nodes > 0)
        {
            int* dead_pids = malloc(sizeof(int) * total_nodes);
            int dead_count = 0;
            findDeadProcesses(root, dead_pids, &dead_count);
            for (int i = 0; i < dead_count; i++)
            {
                root = deleteNode(root, dead_pids[i]);
            }
            free(dead_pids);
        }
    }
    return root;
}

void flattenTree(Node* root, Node** array, int* index)
{
    if (root == NULL)
        return;
    
    flattenTree(root->left, array, index);
    
    array[*index] = root;
    (*index)++;
    
    flattenTree(root->right, array, index);
}

int compareCPU(const void* a, const void* b)
{
    Node* nodeA = *(Node**)a;
    Node* nodeB = *(Node**)b;

    if (nodeB->info->cpu_usage > nodeA->info->cpu_usage) return 1;
    if (nodeB->info->cpu_usage < nodeA->info->cpu_usage) return -1;
    
    return 0;
}

int compareMemory(const void* a, const void* b)
{
    Node* nodeA = *(Node**)a;
    Node* nodeB = *(Node**)b;
    
    if (nodeB->info->memory > nodeA->info->memory) return 1;
    if (nodeB->info->memory < nodeA->info->memory) return -1;

    return 0;
}

int printTable(Node* root, char sort_mode, int* selected_row, int reverse_sort, char* search_filter, int is_searching)
{
    // dimension of the terminal
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int max_rows = w.ws_row - 5; 

    // head of the table
    printf("%-8s | %-5s | %-25s | %-7s | %-10s\033[K\n", "PID", "STATE", "PROCESS NAME", "CPU %", "MEMORY USED");
    printf("--------------------------------------------------------------------\n");

    if (root == NULL) return -1;

    int total_nodes = countNodes(root);
    if (total_nodes == 0) return -1;

    // temporary array
    Node** process_array = malloc(sizeof(Node*) * total_nodes);
    if (process_array == NULL) return -1;

    // nodes extraction
    int total_processes = 0;
    flattenTree(root, process_array, &total_processes);

    if (search_filter[0] != '\0')
    {
        int filter_count = 0;
        for (int i = 0; i < total_processes; i++)
        {
            if (strstr(process_array[i]->info->name, search_filter) != NULL)
            {
                process_array[filter_count++] = process_array[i];
            }
        }
        total_processes = filter_count;

    }

    if (sort_mode == 'c')
    {
        qsort(process_array, total_processes, sizeof(Node*), compareCPU);
    }

    if (sort_mode == 'm')
    {
        qsort(process_array, total_processes, sizeof(Node*), compareMemory);
    }

    if (reverse_sort)
    {
        for (int i = 0; i < total_processes / 2; i++)
        {
            Node* temp = process_array[i];
            process_array[i] = process_array[total_processes - 1 - i];
            process_array[total_processes - 1 - i] = temp;
        }
    }

    // print the first max_rows
    int display_count = (total_processes < max_rows) ? total_processes : max_rows;
    
    if (*selected_row < 0) *selected_row = 0;
    if (*selected_row >= display_count && display_count > 0) *selected_row = display_count - 1;
    else if (display_count == 0) *selected_row = 0;

    int current_selected_pid = -1;

    for (int i = 0; i < display_count; i++)
    {
        Node* curr = process_array[i];

        if (i == *selected_row)
        {
            printf("\033[7m"); //reverse colors
            current_selected_pid = curr->info->PID;
        }

        char* state_color = "\033[39m";
        if ( curr->info->state == 'R' ) state_color = "\033[32m";
        else if ( curr->info->state == 'Z' ) state_color = "\033[31m";
        else if ( curr->info->state == 'S' ) state_color = "\033[33m";
        else state_color = "\033[36m";

        printf("%-8d | %s%c\033[39m    | %-25.25s | %5.1f %% | %7.1f MB\033[K\n", curr->info->PID, state_color, curr->info->state, curr->info->name, curr->info->cpu_usage, curr->info->memory);
    
        if (i == *selected_row)
        {
            printf("\033[0m");
        }
    }

    printf("\033[J");

    printf("\n");
    if (is_searching)
    {
        printf("\033[7m Search: %s \033[0m\033[K\n", search_filter);
    }
    else if (search_filter[0] != '\0')
    {
        printf("Filter active (press '/' to change): %s \033[K\n", search_filter);
    }

    free(process_array);
    return current_selected_pid;
}

void freeTree(Node* root)
{
    if (root == NULL)
    {
        return;
    }
    freeTree(root->left);
    freeTree(root->right);
    free(root->info->name);
    free(root->info);
    free(root);
}