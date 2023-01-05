
all: file_manager file_client

file_manager: file_manager.c 
	gcc file_manager.c -o file_manager -lpthread

file_client: file_client.c 
	gcc file_client.c -o file_client -lpthread

clean:
	rm file_manager file_client