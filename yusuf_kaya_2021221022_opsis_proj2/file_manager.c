#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#define FIFO_NAME_PREFIX "named_pipe_" // Named pipes will be named as named_pipe_1, named_pipe_2, etc.
#define NUM_WORKER_THREADS 5

// Structure to store information about a file
typedef struct file_info
{
    char name[256];
    int size;
} FileInfo;

// Structure to store a request
typedef struct request
{
    char buffer[1024]; // Request data
    struct request *next; // Pointer to next request in queue
} Request;

// Global variables
pthread_mutex_t queue_mutex; // Mutex to protect access to request queue
pthread_cond_t queue_cond; // Condition variable to signal worker threads
pthread_mutex_t file_list_mutex; // Mutex to protect access to File_List
Request *request_queue; // Pointer to first request in queue
FileInfo File_List[1024]; // List of files

// Function prototypes
void *worker_thread_func(void *arg);
void create_file(char *name, int size);
void delete_file(char *name);
void read_file(char *name);
void write_file(char *name, int size);
void add_request(char *buffer);
Request *get_request();

int main()
{
    int i;
    pthread_t worker_threads[NUM_WORKER_THREADS];

    // Initialize mutexes and condition variable
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_cond_init(&queue_cond, NULL);
    pthread_mutex_init(&file_list_mutex, NULL);

    // Initialize File_List
    for (i = 0; i < 1024; i++)
    {
        File_List[i].name[0] = '\0';
        File_List[i].size = 0;
    }

    // Create worker threads
    for (i = 0; i < NUM_WORKER_THREADS; i++)
    {
        if (pthread_create(&worker_threads[i], NULL, worker_thread_func, NULL) != 0)
        {
            perror("Error creating worker thread");
            exit(1);
        }
    }

    while (1)
    {
        // Check for incoming requests
        for (i = 1; i <= NUM_WORKER_THREADS; i++)
        {
            // Open named pipe for reading
            char fifo_name[256];
            sprintf(fifo_name, "%s%d", FIFO_NAME_PREFIX, i); // Generate name for named pipe
            mkfifo(fifo_name,0666);
            int fd = open(fifo_name, O_RDONLY);
            if (fd < 0)
            {
                continue;
            }

            // Read request from named pipe
            char buffer[1024];
            if (read(fd, buffer, 1024) > 0)
            {
                add_request(buffer); // Add request to queue
            }

            close(fd); // Close named pipe
        }
    }

    return 0;
}

void *worker_thread_func(void *arg)
{
    while (1)
    {
        // Wait for request to be available
        pthread_mutex_lock(&queue_mutex);
        while (request_queue == NULL)
        {
            pthread_cond_wait(&queue_cond, &queue_mutex);
        }
        Request *request = get_request(); // Get request from queue
        pthread_mutex_unlock(&queue_mutex);

        // Parse command
        char *command = strtok(request->buffer, " ");
        char *name = strtok(NULL, " ");
        int size = atoi(strtok(NULL, " "));

        // Execute command
        if (strcmp(command, "create") == 0)
        {
            create_file(name, size);
        }
        else if (strcmp(command, "delete") == 0)
        {
            delete_file(name);
        }
                else if (strcmp(command, "read") == 0)
        {
            read_file(name);
        }
        else if (strcmp(command, "write") == 0)
        {
            write_file(name, size);
        }

        free(request); // Free request memory
    }

    return NULL;
}

void create_file(char *name, int size)
{
   
    // Lock mutex
    pthread_mutex_lock(&file_list_mutex);

    // Add file to list
    int i;
    for (i = 0; i < 1024; i++)
    {
        if (File_List[i].name[0] == '\0')
        {
            strcpy(File_List[i].name, name);
            File_List[i].size = size;
            break;
        }
    }

    // Unlock mutex
    pthread_mutex_unlock(&file_list_mutex);
}

void delete_file(char *name)
{
    // Lock mutex
    pthread_mutex_lock(&file_list_mutex);

    // Delete file from list
    int i;
    for (i = 0; i < 1024; i++)
    {
        if (strcmp(File_List[i].name, name) == 0)
        {
            File_List[i].name[0] = '\0';
            File_List[i].size = 0;
            break;
        }
    }

    // Unlock mutex
    pthread_mutex_unlock(&file_list_mutex);
}

void read_file(char *name)
{
        // Lock mutex
    pthread_mutex_lock(&file_list_mutex);

    // Read file from list
    int i;
    for (i = 0; i < 1024; i++)
    {
        if (strcmp(File_List[i].name, name) == 0)
        {
            printf("File %s has size %d\n", name, File_List[i].size);
            break;
        }
    }

    // Unlock mutex
    pthread_mutex_unlock(&file_list_mutex);
}

void write_file(char *name, int size)
{
    // Lock mutex
    pthread_mutex_lock(&file_list_mutex);

    // Write to file in list
    int i;
    for (i = 0; i < 1024; i++)
    {
        if (strcmp(File_List[i].name, name) == 0)
        {
            File_List[i].size = size;
            break;
        }
    }

    // Unlock mutex
    pthread_mutex_unlock(&file_list_mutex);
}

void add_request(char *buffer)
{
    // Allocate memory for request
    Request *request = (Request *)malloc(sizeof(Request));
    strcpy(request->buffer, buffer);
    request->next = NULL;

    // Add request to queue
    pthread_mutex_lock(&queue_mutex);
    if (request_queue == NULL)
    {
        request_queue = request;
    }
    else
    {
        Request *current = request_queue;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = request;
    }
    pthread_cond_signal(&queue_cond); // Signal worker threads that a new request is available
    pthread_mutex_unlock(&queue_mutex);
}
Request *get_request()
{
    Request *request = request_queue;
    request_queue = request_queue->next;
    return request;
}




