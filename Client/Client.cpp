#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN


#include "Client.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27016
#define BUFFER_SIZE 256


SOCKET Connect(char* queueName) {

	// Socket used to communicate with server
	SOCKET connectSocket = INVALID_SOCKET;

	// Variable used to store function return value
	int iResult;

	//// Buffer we will use to store message
	//char dataBuffer[BUFFER_SIZE];

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return NULL;
	}

	// create a socket
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return NULL;
	}

	// Create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;								// IPv4 protocol
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
	serverAddress.sin_port = htons(SERVER_PORT);					// server port

	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
		return NULL;
	}

	// Send message to server using connected socket
	iResult = send(connectSocket, queueName, (int)strlen(queueName) + 1, 0);
	// Check result of send function
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return NULL;
	}

	printf("Message successfully sent. Total bytes: %ld\n", iResult);

	return connectSocket;
}

int SendMsg(char* queueName, char* message, int messageSize,SOCKET* connectSocket) {

	Message* msg = (Message*)malloc(sizeof(Message));
	strcpy(msg->destination, queueName);
	strcpy(msg->message_content, message);
	msg->size_of_message= htonl((unsigned long)messageSize);

	// Send message to server using connected socket
	int iResult = send(*connectSocket, (char*)(msg), sizeof(Message), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(*connectSocket);
		WSACleanup();
		return 1;
	}

	free(msg);
	return iResult;	
}



// TCP client that use blocking sockets
int main() 
{	
	int iResult;
	printf("Enter username for your message services: ");
	char msg[MAX_SIZE_NAME];
	gets_s(msg, MAX_SIZE_NAME);

	SOCKET connectSocket = Connect(msg);
	if (connectSocket == NULL)
	{
		printf("Error with connectiong to a Server: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	printf("Connected to server and DB\n");


	while (true)
	{
		printf("Choose option: \n\n");
		printf("1: Send message\n");
		printf("2: View messages\n");
		printf("3: Exit\n");

		char option[5];
		gets_s(option, 5);

		if (strcmp(option,"1")==0)
		{
			Message* msg = (Message*)malloc(sizeof(Message));
			printf("Enter destination: ");			
			gets_s(msg->destination, MAX_SIZE_NAME);
			printf("\n");
			printf("Enter message: ");
			char tmp[MAX_MESSAGE_SIZE];		
			gets_s(tmp, MAX_MESSAGE_SIZE);
			strcpy(msg->message_content, tmp);
			msg->size_of_message = strlen(tmp);
			int iResult = SendMsg(msg->destination, msg->message_content, msg->size_of_message, &connectSocket);
			free(msg);
			if (iResult == SOCKET_ERROR)
			{
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
				WSACleanup();
				return 1;
			}
		}
		else if (strcmp(option, "2") == 0)
		{

			Message* msg = (Message*)malloc(sizeof(Message));
			strcpy(msg->destination, "server");
			strcpy(msg->message_content, "send my inbox");
			
			int iResult = send(connectSocket, (char*)(msg), sizeof(Message), 0);
			if (iResult == SOCKET_ERROR)
			{
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
				WSACleanup();
				return 1;
			}

			free(msg);



			    Message* msgRec = (Message*)malloc(sizeof(Message));
									
				while (true)
				{
					iResult = recv(connectSocket, (char*)msgRec, sizeof(Message), 0);
					if (iResult > 0)
					{
						printf("MESSAGE\n\n");
						printf("FROM: %s\n TEXT: %s\n\n", msgRec->source, msgRec->message_content);
						if (strcmp(msgRec->destination, "client") == 0, strcmp(msgRec->message_content, "all done") == 0) {
							printf("All delivered, press any key to go back\n");
							_getch();
							free(msgRec);
							break;
						}
					}
					else if (iResult == 0)	// Check if shutdown command is received
					{
						// Connection was closed successfully
						printf("Connection with server closed.\n");
						closesocket(connectSocket);
						WSACleanup();


					}
					else	// There was an error during recv
					{
						printf("recv failed with error: %d\n", WSAGetLastError());
						closesocket(connectSocket);
						WSACleanup();
					}

				}
			

		}
		else if (strcmp(option, "3") == 0)//gasi
		{

			// Shutdown the connection since we're done
			iResult = shutdown(connectSocket, SD_SEND); //da li ce mi ugasiti i server?

			// Check if connection is succesfully shut down.
			if (iResult == SOCKET_ERROR)
			{
				printf("Shutdown failed with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
				WSACleanup();
				return 1;
			}

			// Close connected socket
			closesocket(connectSocket);

			// Deinitialize WSA library
			WSACleanup();

			return 0;
		}
	}


    return 0;
}