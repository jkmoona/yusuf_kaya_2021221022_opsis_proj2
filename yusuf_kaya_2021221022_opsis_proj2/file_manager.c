#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 100
#define NUM_THREADS 2
#define FILE_LIST_SIZE 10
#define QUEUE_SIZE 20

typedef struct
{
    char command[BUFFER_SIZE];
    char file_name[BUFFER_SIZE];
    char file_contents[BUFFER_SIZE];
} request_t;

// File information data structure
typedef struct
{
    char file_name[BUFFER_SIZE];
    char file_contents[BUFFER_SIZE];
    int in_use;
    int fd;
    pthread_mutex_t mutex;
} file_t;

// File list data structure
file_t File_List[FILE_LIST_SIZE];
pthread_mutex_t file_list_mutex;

// Request queue data structure
typedef struct
{
    request_t requests[QUEUE_SIZE];
    int front;
    int rear;
    int size;
} request_queue_t;
pthread_mutex_t queue_lock;
pthread_cond_t queue_cond;

// Initialize the request queue
request_queue_t request_queue;

void init_request_queue()
{
    request_queue.front = 0;
    request_queue.rear = -1;
    request_queue.size = 0;
    pthread_mutex_init(&queue_lock, NULL);
}

// Enqueue a request to the request queue
void enqueue_request(request_t request)
{
    pthread_mutex_lock(&queue_lock);

    // Add the request to the queue
    request_queue.rear = (request_queue.rear + 1) % QUEUE_SIZE;
    request_queue.requests[request_queue.rear] = request;
    request_queue.size++;
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_lock);
}

// Dequeue a request from the request queue
request_t dequeue_request()
{
    pthread_mutex_lock(&queue_lock);

    // Remove a request from the queue
    request_t request = request_queue.requests[request_queue.front];
    request_queue.front = (request_queue.front + 1) % QUEUE_SIZE;
    request_queue.size--;

    pthread_mutex_unlock(&queue_lock);

    return request;
}

int queue_empty()
{
    return request_queue.rear == -1;
}
int queue_full() {
    if (request_queue.front == (request_queue.rear + 1) % QUEUE_SIZE) {
        return 1;
    }
    return 0;
}

void create_file(char *filename)
{
    // Check if file already exists in File_List
    int i;
    for (i = 0; i < 10; i++)
    {
        if (strcmp(File_List[i].file_name, filename) == 0)
        {
            printf("Error: file already exists\n");
            exit(1);
        }
    }

    // Find the first empty slot in File_List
    for (i = 0; i < 10; i++)
    {
        if (File_List[i].file_name[0] == '\0')
        {
            strcpy(File_List[i].file_name, filename);
            puts("Found a spot!");
            break;
        }
    }

    // Check if File_List is full
    if (i == 10)
    {
        printf("Error: File_List is full\n");
        return;
    }

    // Create the file on the system
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("Error: failed to create file\n");
        return;
    }
    fclose(fp);
}

void delete_file(char *filename)
{
    // Check if file exists in File_List
    int i;
    for (i = 0; i < 10; i++)
    {
        if (strcmp(File_List[i].file_name, filename) == 0)
        {
            break;
        }
    }

    // Check if file does not exist in File_List
    if (i == 10)
    {
        printf("Error: file does not exist\n");
        return;
    }

    // Delete the file from the system
    if (remove(filename) != 0)
    {
        printf("Error: failed to delete file\n");
        return;
    }

    // Clear the file's entry in File_List
    memset(File_List[i].file_name, 0, sizeof(File_List[i].file_name));
    memset(File_List[i].file_contents, 0, sizeof(File_List[i].file_contents));
}

void read_file(char *filename)
{
    // Check if file exists in File_List
    int i;
    for (i = 0; i < 10; i++)
    {
        if (strcmp(File_List[i].file_name, filename) == 0)
        {
            break;
        }
    }

    // Check if file does not exist in File_List
    if (i == 10)
    {
        printf("Error: file does not exist\n");
        return;
    }

    // Read from the file and store its contents in File_List
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error: failed to read file\n");
        return;
    }
    fgets(File_List[i].file_contents, BUFFER_SIZE, fp);
    fclose(fp);
}

void write_file(request_t request)
{
    int i;
    for (i = 0; i < FILE_LIST_SIZE; i++)
    {
        if (strcmp(File_List[i].file_name, request.file_name) == 0)
        {
            // Lock the file
            pthread_mutex_lock(&File_List[i].mutex);

            // Open the file in write mode
            FILE *file = fopen(request.file_name, "w");
            if (file == NULL)
            {
                printf("Error opening file!\n");
                exit(1);
            }

            // Write to the file
            fprintf(file, "%s", request.file_contents);

            // Close the file
            fclose(file);

            // Update the contents in File_List
            strcpy(File_List[i].file_contents, request.file_contents);

            // Unlock the file
            pthread_mutex_unlock(&File_List[i].mutex);
            break;
        }
    }
}

// Worker thread function
void *worker_thread_func(void *arg)
{
    request_t request;
    while (1)
    {
        pthread_mutex_lock(&queue_lock);
        while (queue_empty())
        {
            // TODO 
            // Workers are not getting in this code block
            // exit(1);
            pthread_cond_wait(&queue_cond, &queue_lock);
        }
        request = dequeue_request();
        // puts("Request dequeued!");
        pthread_mutex_unlock(&queue_lock);

        if (strcmp(request.command, "CREATE_FILE") == 0)
        {
            pthread_mutex_lock(&file_list_mutex);
            create_file(request.file_name);
            pthread_mutex_unlock(&file_list_mutex);
        }
        else if (strcmp(request.command, "DELETE_FILE") == 0)
        {

            pthread_mutex_lock(&file_list_mutex);
            delete_file(request.file_name);
            pthread_mutex_unlock(&file_list_mutex);
        }
        else if (strcmp(request.command, "READ_FILE") == 0)
        {

            pthread_mutex_lock(&file_list_mutex);
            read_file(request.file_name);
            pthread_mutex_unlock(&file_list_mutex);
        }
        else if (strcmp(request.command, "WRITE_FILE") == 0)
        {

            pthread_mutex_lock(&file_list_mutex);
            write_file(request);
            pthread_mutex_unlock(&file_list_mutex);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    // Create thread pool for worker threads
    pthread_t worker_threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&worker_threads[i], NULL, worker_thread_func, NULL);
    }

    // Create named pipe
    char *pipe_name = "/tmp/manager_pipe";
    mkfifo(pipe_name, 0666);

    // Open named pipe
    int pipe_fd = open(pipe_name, O_RDONLY);
    if (pipe_fd == -1)
    {
        perror("Error! Can't open pipe");
        return 1;
    }

    request_t request;
    while (1)
    {
        // Read a request from the pipe
        read(pipe_fd, &request, sizeof(request_t));
        printf("\nrequest = %s - %s\n", request.command, request.file_name);

        // Enqueue the request
        pthread_mutex_lock(&queue_lock);
        enqueue_request(request);
        puts("Request queued!");
        pthread_cond_signal(&queue_cond);
        pthread_mutex_unlock(&queue_lock);
    }

    // Close named pipe
    close(pipe_fd);
    unlink(pipe_name);

    // Wait for worker threads to complete
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(worker_threads[i], NULL);
    }

    return 0;
}
