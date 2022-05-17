#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>

int main()
{
	int clientSocket;
	int status;
	int connected;
	int end= 1;
	char quit[256] = "QUIT";
	char push[256] = "PUSH";
	char pull[256] = "PULL";
	char endOfFile[256] = "END";
	char response[256];
	char userInput[256];
	char buffer[256];
	int fileSize;
	char *filename1 = "/Users/daniel/Desktop/testCopy.txt";
	char *filename2 = "/Users/daniel/Desktop/testPull.txt";
	FILE *file;
	struct sockaddr_in address;
	socklen_t addr_size;
	char filename[256];
	char test[256];
	char size[256] = "fileSize";
	char byte1[256] = " <<<";
	char byte2[256] = ">>>";
	char quote1[256] = " ";
	char quote2[256] = " ";
	char info[256];
	int j;
	char i[256];

	struct stat fileStats;

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (clientSocket == -1)
	{
		printf("Error creating socket...\n");
	}
	else
	{
		printf("Socket created...\n");
	}

	address.sin_family = AF_INET;
	address.sin_port = htons(9999);
	address.sin_addr.s_addr = INADDR_ANY;

	connected = connect(clientSocket, (struct sockaddr *) &address, sizeof(address));

	if (connected == -1)
	{
		printf("Error connecting...\n");
	}
	else
	{
		printf("Connected to server...\n");
		while (1)
		{
			printf("Enter PUSH, PULL, or QUIT: \n");
			scanf("%s", userInput);
			if (strcmp(userInput, quit) == 0)
			{
				send(clientSocket, userInput, sizeof(userInput), 0);
				printf("Quitting...\n");
				exit(1);
			}
			else
			{
				if (strcmp(userInput, push) == 0)
				{
					printf("Enter filename: ");
					scanf("%s", filename);
					if (stat(filename, &fileStats) == 0)
					{
						j = fileStats.st_size;
						sprintf(i, "%d", j);
					}
					strcat(userInput, quote1);
					strcat(userInput, filename);
					strcat(userInput, quote2);
					strcat(userInput, size);
					strcat(userInput, byte1);
					strcat(userInput,  i);
					strcat(userInput, byte2);
					printf("%s\n", userInput);
					status = send(clientSocket, userInput, sizeof(userInput), 0);
					file = fopen(filename, "r");
					while(fgets(buffer, sizeof(buffer), file) != NULL)
					{
						status = send(clientSocket, buffer, sizeof(buffer), 0);
					}
					fclose(file);
					send(clientSocket, endOfFile, sizeof(endOfFile), 0);
					if (status == -1)
					{
						printf("Error.\n");
					}
					else
					{
						if(recv(clientSocket, &response, sizeof(response), 0) == -1)
						{
							printf("Error\n");
						}
						else
						{
							printf("Server response: %s\n", response);
						}
					}
				}
				else if (strcmp(userInput, pull) == 0)
				{
					printf("Enter filename: ");
					scanf("%s", filename);
					strcat(userInput, quote1);
					strcat(userInput, filename);
					strcat(userInput, "");
					strcat(userInput, "");
					send(clientSocket, userInput, sizeof(userInput), 0);
					file = fopen(filename1, "w");
					while (end > 0)
					{
						end = recv(clientSocket, &buffer, sizeof(buffer), 0);
						if (end == -1)
						{
							printf("Error.\n");
						}
						else
						{
							if (strcmp(endOfFile, buffer) == 0)
							{
								break;
							}
							else
							{
								fputs(buffer, file);
							}
						}
					}
					fclose(file);

					if(recv(clientSocket, &fileSize, sizeof(fileSize), 0) == -1)
					{
						printf("Error\n");
					}
					else
					{
						printf("File Size: %i\n", fileSize);
					}

					if(recv(clientSocket, &response, sizeof(response), 0) == -1)
					{
						printf("Error\n");
					}
					else
					{
						printf("Server response: %s\n", response);
					}
				}
			}
		}
	}

	return 0;
}
