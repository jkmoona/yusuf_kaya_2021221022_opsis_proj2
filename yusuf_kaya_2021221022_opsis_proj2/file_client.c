#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FIFO_NAME_PREFIX "/tmp/named_pipe_" // Named pipes will be named as named_pipe_1, named_pipe_2, etc.

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: file_client client_id\n");
        exit(1);
    }

    // Get client ID
    int client_id = atoi(argv[1]);

    // Open named pipe for writing
    char fifo_name[256];
    sprintf(fifo_name, "%s%d", FIFO_NAME_PREFIX, client_id); // Generate name for named pipe
    mkfifo(fifo_name,0666);
    int fd = open(fifo_name, O_RDWR);
    if (fd < 0)
    {
        perror("Error opening named pipe");
        exit(1);
    }

    // Read commands from user and send them to file_manager
    while (1)
    {
        // Print options
        printf("1. Create file\n");
        printf("2. Delete file\n");
        printf("3. Read file\n");
        printf("4. Write to file\n");
        printf("5. Exit\n");
        printf("Enter option: ");

        int option;
        scanf("%d", &option);

        if (option == 1)
        {
            // Create file
            char buffer[1024];
            printf("Enter file name: ");
            scanf("%s", buffer);
            strcat(buffer, " ");
            printf("Enter file size: ");
            int size;
            scanf("%d", &size);
            char size_str[16];
            sprintf(size_str, "%d", size);
            strcat(buffer, size_str);
            write(fd, buffer, strlen(buffer));
        }
        else if (option == 2)
        {
            // Delete file
            char buffer[1024];
            printf("Enter file name: ");
            scanf("%s", buffer);
            write(fd, buffer, strlen(buffer));
        }
        else if (option == 3)
        {
            // Read file
            char buffer[1024];
            printf("Enter file name: ");
            scanf("%s", buffer);
            write(fd, buffer, strlen(buffer));
        }
        else if (option == 4)
        {
            // Write to file
            char buffer[1024];
            printf("Enter file name: ");
            scanf("%s", buffer);
            strcat(buffer, " ");
            printf("Enter file content: ");
            char content[1024];
            scanf("%s", content);
            strcat(buffer, content);
            write(fd, buffer, strlen(buffer));
        }
        else if (option == 5)
        {
            // Exit
            break;
        }
        else
        {
            printf("Invalid option\n");
        }
    }

    close(fd); // Close named pipe

    return 0;
}
