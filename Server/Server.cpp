#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#include "Server.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define MAX_SOCKETS 6
#define SERVER_PORT 27016
#define BUFFER_SIZE 256


int currentClients = 0; 
int ConnectionThreadFlag = 0;


void Compact(SOCKET* acceptedSockets,int startIndex, int currentSize) {
	for (int i = startIndex; i < currentSize - 1; i++) {
		acceptedSockets[i] = acceptedSockets[i + 1];
	}
}


DWORD WINAPI ConnectionThread(LPVOID lpParam)
{
	SOCKET* listenSocket = (SOCKET *)lpParam;
	// Socket used for communication with client
	SOCKET acceptedSocket[MAX_SOCKETS];

	DWORD exitCode = 2;

	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		acceptedSocket[i] = INVALID_SOCKET;
	}

	fd_set readfds;
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;


	sockaddr_in client;
	int size = sizeof(client);

	FD_ZERO(&readfds);

	while (true)
	{

		if (currentClients != MAX_SOCKETS)
		{
			FD_SET(*listenSocket, &readfds);
		}

		int iResult = select(0, &readfds, NULL, NULL, &timeVal);

		if (iResult == 0) {
			continue;
		}
		else if (iResult == SOCKET_ERROR)
		{
			printf("error");
			closesocket(*listenSocket);
			WSACleanup();
			return -1;
		}
		else if (FD_ISSET(*listenSocket, &readfds)) { // Connection request - username sent
			printf("Client ");
			acceptedSocket[currentClients] = accept(*listenSocket, (SOCKADDR*)&client, &size);

			if (acceptedSocket[currentClients] == INVALID_SOCKET)
			{
				closesocket(acceptedSocket[currentClients]);
				printf("Client connection error %d", currentClients);
				continue;
			}

			DWORD communicaton;
			HANDLE threadHandler;
			threadHandler = CreateThread(NULL, 0, &Communication_with_client, &acceptedSocket[currentClients], 0, &communicaton);
			if (threadHandler == NULL)
			{
				printf("Error with creating a thread\n");
			}

			currentClients++;
		}

	}
	
}

DWORD WINAPI Move_from_outbox_to_inbox(LPVOID lpParam)
{
	char username[MAX_SIZE_NAME];
	strcpy(username, (char*)lpParam);
	TableValue* tv = hash_table_retreive(username);
	while (true)
	{
		Message* msg = Dequeue(&(tv->outbox_start));
		if (msg == NULL)
		{
			Sleep(1500);
			continue;
		}

		insert_message_in(msg->destination, *msg, "in");
	
		free(msg);
		print_table();
	}

}
DWORD WINAPI Communication_with_client(LPVOID lpParam)
{
	int iResult;
	DWORD exitCode = 2;
	
	TableValue *tv = (TableValue*)malloc(sizeof(TableValue));

	if (tv == NULL) {
		printf("Not enough RAM!\n");
		exit(21);
	}

	bool firstTimeEnter = true;
	SOCKET* acceptedSocket = (SOCKET *)lpParam;
	unsigned long mode = 0;
	iResult = ioctlsocket(*acceptedSocket, FIONBIO, &mode); // Block regime
	char username[MAX_SIZE_NAME];
	DWORD transfer;
	HANDLE threadHandler=NULL; 

	while (true)
	{

		Message* msg = (Message*)malloc(sizeof(Message));

		if (msg == NULL) {
		printf("Not enough RAM!\n");
		exit(21);
		}

		if (firstTimeEnter == true)// For first message that is username
		{
			
			iResult = recv(*acceptedSocket, username, MAX_SIZE_NAME, 0);
			printf("%s \n", username);

			tv = hash_table_retreive(username);
			
			if (tv == NULL)
			{
				hash_table_insert_client(username);
			}
		
			threadHandler = CreateThread(NULL, 0, &Move_from_outbox_to_inbox, username, 0, &transfer);
			if (threadHandler == NULL)
			{
				printf("Error with creating a thread for transfering from outbox to inbox\n");
			}

			firstTimeEnter = false;
		}

		// Waiting to receive data until the client shuts down the connection
		iResult = recv(*acceptedSocket, (char*)msg, sizeof(Message), 0);

		if (iResult > 0)	// Check if message is successfully received
		{
			bool writeDB = true;
			if (strcmp(msg->destination, "server") == 0 && strcmp(msg->message_content, "send my inbox") == 0)
			{
				TableValue* tv = hash_table_retreive(username);
				while (true) {
					Message* msgSend = Dequeue(&(tv->inbox_start));
					if (msgSend == NULL)
					{
						Message* endMsg = (Message*)malloc(sizeof(Message));
						strcpy(endMsg->destination, "client");
						strcpy(endMsg->source, "mail server");
						strcpy(endMsg->message_content, "all done");


						int iResult = send(*acceptedSocket, (char*)(endMsg), sizeof(Message), 0);
						if (iResult == SOCKET_ERROR)
						{
							printf("send failed with error: %d\n", WSAGetLastError());
							closesocket(*acceptedSocket);
							WSACleanup();
							return 1;
						}

						free(endMsg);
						free(msgSend);
						writeDB = false;
						break;
					}
						
					int iResult = send(*acceptedSocket, (char*)(msgSend), sizeof(Message), 0);
					if (iResult == SOCKET_ERROR)
					{
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(*acceptedSocket);
						WSACleanup();
						return 1;
					}
				}				
			}

			if (writeDB)
			{
				msg->size_of_message = ntohl((unsigned long)msg->size_of_message);
				strcpy(msg->source, username);

				printf("Client %s sended to: %s message: %s lenght: %d.\n", msg->source, msg->destination, msg->message_content, msg->size_of_message);

				insert_message_in(username, *msg, "out");
			}
		}
		else if (iResult == 0)	// Check if shutdown command is received
		{
			// Connection was closed successfully
			printf("Connection with client closed.\n");
			currentClients--;
			free(msg);
			free(tv);
			if (SOCKET_ERROR == closesocket(*acceptedSocket))
			{
				printf("error with closing socket");
			}
			
			TerminateThread(threadHandler, exitCode);
			ExitThread(exitCode);
		
		}
		else // There was an error during recv
		{
			printf("recv failed with error: %d\n", WSAGetLastError());
			currentClients--;
			free(msg);
			free(tv);
			if (SOCKET_ERROR == closesocket(*acceptedSocket))
			{
				printf("error with closing socket");
			}

			TerminateThread(threadHandler, exitCode);
			ExitThread(exitCode);
		}		
	}
}

// TCP server that use blocking sockets
int main()
{
	//Initialize HashTable
	init_hash_table();

	// Socket used for listening for new clients 
	SOCKET listenSocket = INVALID_SOCKET;

	// Variable used to store function return value
	int iResult;

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	// Initialize serverAddress structure used by bind
	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;				// IPv4 address family
	serverAddress.sin_addr.s_addr = INADDR_ANY;		// Use all available addresses
	serverAddress.sin_port = htons(SERVER_PORT);	// Use specific port

	// Create a SOCKET for connecting to server
	listenSocket = socket(AF_INET,      // IPv4 address family
		SOCK_STREAM,  // Stream socket
		IPPROTO_TCP); // TCP protocol

	// Check if socket is successfully created
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket - bind port number and local address to socket
	iResult = bind(listenSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));

	// Check if socket is successfully binded to address and port from sockaddr_in structure
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	unsigned long mode = 1;
	// Socket in non-block regime
	iResult = ioctlsocket(listenSocket, FIONBIO, &mode);
	if (iResult == SOCKET_ERROR)
	{
		closesocket(listenSocket);
		WSACleanup();
		return -1;
	}

	// Set listenSocket in listening mode
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Server socket is set to listening mode. Waiting for new connection requests.\n");

	DWORD connection;
	HANDLE threadHandler;
	threadHandler = CreateThread(NULL, 0, &ConnectionThread, &listenSocket, 0, &connection);
	if (threadHandler == NULL)
	{
		printf("Error with creating a thread\n");
	}

	LPDWORD exitCode = NULL;

		while(true)
		{

			if (ConnectionThreadFlag == -1)
			{
				break;
			}
	
		}
		CloseHandle(threadHandler);

	//Close listen and accepted sockets
    closesocket(listenSocket);

	free_hash();
	// Deinitialize WSA library
    WSACleanup();
}
	




 
