#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <libgen.h>

// Helper function that checks to see if server was able to successfully listen 
// for a new client connection.
void listening(int check)
{
	if (check == -1)
	{
		printf("Error Listening...\n");
	}
	else
	{
		printf("Listening...\n");
	}
}

// Main server function that creates a TCP Server-Client allowing for 
// remote backup of files.
//
//Takes command line arguments [directory, port]
int main(int argc, char *argv[])
{
	int port;
	int server_socket;
	int status;
	int client;
	int end_of_file = 1;
	int accepted = 0;

	char *directory;
	char client_command[256];
	char client_sent_name[256];
	char size[256];
	char bytes[256];
	char client_sent_info[256];
	char file_text[256];
	char push[256] = "PUSH";
	char pull[256] = "PULL";
	char quit[256] = "QUIT";
	char server_response[256] = "OK";
	char end[256] = "END";

	struct sockaddr_in socket_info;

	FILE *file_pointer;
	socklen_t addr_size;

	for (int i = 1; i < argc; i++)
	{
		if (i == 1)
		{
			directory = argv[i];
		}
		else if (i == 2)
		{
			port = strtol(argv[i], NULL, 10);
		}
	}

	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (server_socket == -1)
	{
		printf("Error creating socket...\n");
	}
	else
	{
		printf("Socket created...\n");
	}

	socket_info.sin_family = AF_INET;
	socket_info.sin_port = htons(port);
	socket_info.sin_addr.s_addr = INADDR_ANY;

	status = bind(server_socket, (struct sockaddr *) &socket_info, sizeof(socket_info));

	if (status == -1)
	{
		printf("Error binding...\n");
	}
	else
	{
		status = listen(server_socket, 5);
		listening(status);

		// Accept loop that allows for multiple clients to communicate with server one 
		// at a time.
		while (1)
		{
			if (accepted == 0)
			{
				client = accept(server_socket, (struct sockaddr *) &socket_info, &addr_size);
				accepted = 1;
			}

			if (client == -1)
			{
				printf("Error accepting from client...\n");
			}
			else
			{
				// Receive command from client and file info, then extract filename from path
				// and append it to the directory specified in command line argument.
				recv(client, &client_sent_info, sizeof(client_sent_info), 0);
				sscanf(client_sent_info, "%s %s %s %s", client_command, client_sent_name, size, bytes);
				char *file_name = client_sent_name;

				// Writes client sent file to specified location and responds to client.
				if (strcmp(push, client_command) == 0)
				{	
					printf("Client command: PUSH\n");
					file_pointer = fopen(file_name, "w");

					while (end_of_file  > 0)
					{
						end_of_file  = recv(client, &file_text, sizeof(file_text), 0);

						if (strcmp(end, file_text) == 0)
						{
							break;
						}
						else
						{
							fputs(file_text, file_pointer);
						}
					}

					fclose(file_pointer);
					send(client, server_response, sizeof(server_response), 0);
					printf("Saving file...\n");
					printf("Waiting...\n");
				}
				// Reads client specified file stored on server and sends it to client along
				// with file size and server response.
				else if (strcmp(pull, client_command) == 0)
				{
					struct stat file_stats;
					printf("Client command: PULL\n");
					printf("Retrieving file...\n");
					file_pointer = fopen(file_name, "r");

					while(fgets(file_text, sizeof(file_text), file_pointer) != NULL)
					{
						send(client, file_text, sizeof(file_text), 0);
					}

					fclose(file_pointer);
					send(client, end, sizeof(end), 0);

					if (stat(file_name, &file_stats) == 0)
					{
						int file_size = file_stats.st_size;
						send(client, &file_size, sizeof(file_size), 0);
					}

					send(client, server_response, sizeof(server_response), 0);
					printf("Waiting...\n");
				}
				// Starts accept loop over and listens for additional clients looking 
				// to connect to the server.
				else if (strcmp(quit, client_command) == 0)
				{
					printf("Client command: QUIT\n");
					status = listen(server_socket, 5);
					accepted = 0;
					listening(status);
				}
				else
				{
					printf("Fatal Error:\n");
					printf("Shutting down...\n");
					exit(1);
				}
			}
		}
	}

	return 0;
}
