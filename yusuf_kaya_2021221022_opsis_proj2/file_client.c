#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 100

typedef struct
{
    char command[BUFFER_SIZE];
    char file_name[BUFFER_SIZE];
    char file_contents[BUFFER_SIZE];
} request_t;

int main(int argc, char *argv[])
{
    // Open the named pipe
    char *pipe_name = "/tmp/manager_pipe";
    mkfifo(pipe_name, 0666);
    int pipe_fd = open(pipe_name, O_WRONLY);
    if (pipe_fd < 0)
    {
        printf("Error opening named pipe\n");
        return 1;
    }

    // Loop until the user exits
    int choice;
    while (1)
    {
        // Print the menu
        printf("1. Create file\n");
        printf("2. Delete file\n");
        printf("3. Read file\n");
        printf("4. Write to file\n");
        printf("5. Exit\n");

        // Read the user's choice
        choice = 0;
        printf("Enter choice: ");
        scanf("%d", &choice);

        // Create a request
        request_t request;

        // Perform the requested action
        switch (choice)
        {
        case 1:
            // Create a file
            printf("Enter file name: ");
            scanf("%s", request.file_name);
            strcpy(request.command, "CREATE_FILE");
            write(pipe_fd, &request, sizeof(request_t));
            printf("\nrequest = %s - %s\n", request.command, request.file_name);
            break;
        case 2:
            // Delete a file
            printf("Enter file name: ");
            scanf("%s", request.file_name);
            strcpy(request.command, "DELETE_FILE");
            write(pipe_fd, &request, sizeof(request_t));
            break;
        case 3:
            // Read a file
            printf("Enter file name: ");
            scanf("%s", request.file_name);
            strcpy(request.command, "READ_FILE");
            write(pipe_fd, &request, sizeof(request_t));
            break;
        case 4:
            // Write to a file
            printf("Enter file name: ");
            scanf("%s", request.file_name);
            printf("Enter file contents: ");
            scanf("%s", request.file_contents);
            strcpy(request.command, "WRITE_FILE");
            write(pipe_fd, &request, sizeof(request_t));
            break;
        case 5:
            // Exit
            strcpy(request.command, "EXIT");
            write(pipe_fd, &request, sizeof(request_t));
            close(pipe_fd);
            return 0;
        default:
            printf("Invalid choice\n");
            break;
        }
    }

    return 0;
}
