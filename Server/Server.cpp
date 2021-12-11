#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#include "Server.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_PORT 27016
#define BUFFER_SIZE 256
#define MAX_SOCKETS 6

int currentClients = 0; 


void Compact(SOCKET* acceptedSockets,int startIndex, int currentSize) {
	for (int i = startIndex; i < currentSize - 1; i++) {
		acceptedSockets[i] = acceptedSockets[i + 1];
	}
}

DWORD WINAPI Move_from_outbox_to_inbox(LPVOID lpParam)
{
	char username[MAX_SIZE_NAME];
	strcpy(username, (char*)lpParam);
	TableValue* tv = hash_table_retreive(username);
	while (true)
	{
		Message* msg = Dequeue(&(tv->outbox_start));//mozda da napravimo neki get message u hash tablu
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
DWORD WINAPI Communication_with_client(LPVOID lpParam)//vrati nesto
{
	//POBLJSANJE SA thread poolom
	int iResult;
	

	TableValue *tv = (TableValue*)malloc(sizeof(TableValue));

	if (tv == NULL) {
		printf("Not enough RAM!\n");
		exit(21);
	}

	bool firstTimeEnter = true;
	SOCKET* acceptedSocket = (SOCKET *)lpParam;
	unsigned long mode = 0;
	iResult = ioctlsocket(*acceptedSocket, FIONBIO, &mode);//blokirajuci rezim
	Message* msg = (Message*)malloc(sizeof(Message));// ovo msm da cak i ne mora u svom projasu sam video da sam samo stavio msg bez alociranja
	char username[MAX_SIZE_NAME];
	DWORD transfer;
	HANDLE threadHandler=NULL; // bez ovoga error

	if (msg == NULL) {
		printf("Not enough RAM!\n");
		exit(21);
	}

	while (true)
	{
		if (firstTimeEnter == true)// za prvu porukicu koja je username
		{
			
			iResult = recv(*acceptedSocket, username, MAX_SIZE_NAME, 0);
			tv = hash_table_retreive(username);
			//to do validacija na klijentu da li je bas ime od 14+1 karakter 
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

		// Receive data until the client shuts down the connection
		iResult = recv(*acceptedSocket, (char*)msg, sizeof(Message), 0);

		if (iResult > 0)	// Check if message is successfully received
		{
			//dataBuffer[iResult] = '\0';
			msg->size_of_message = ntohl((unsigned long)msg->size_of_message);
			strcpy(msg->source, username);
			// Log message text
			printf("Client %s sent: %s %s %d.\n",msg->source, msg->destination, msg->message_content, msg->size_of_message);
			insert_message_in(username, *msg, "out");

		}
		else if (iResult == 0)	// Check if shutdown command is received
		{
			// Connection was closed successfully
			printf("Connection with client closed.\n");
			currentClients--;// deljena promenljiva
			free(msg);
			free(tv);
			closesocket(*acceptedSocket);
		    CloseHandle(threadHandler);
			
			return 0;
		}
		else	// There was an error during recv
		{

			printf("recv failed with error: %d\n", WSAGetLastError());
			currentClients--;
			free(msg);
			free(tv);
			closesocket(*acceptedSocket);
			CloseHandle(threadHandler);
			return 0;
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

	// Socket used for communication with client
	SOCKET acceptedSocket[MAX_SOCKETS];

	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		acceptedSocket[i] = INVALID_SOCKET;
	}

	// Variable used to store function return value
	int iResult;

	//// Buffer used for storing incoming data
	//char dataBuffer[BUFFER_SIZE];

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
			FD_SET(listenSocket, &readfds);
		}

		for (int i = 0; i < currentClients; i++)
		{
			FD_SET(acceptedSocket[i], &readfds);
		}

		iResult = select(0, &readfds, NULL, NULL, &timeVal);

		if (iResult == 0) {
			printf("No changes \n");
			continue;
		}
		else if (iResult == SOCKET_ERROR)
		{
			printf("error");
			closesocket(listenSocket);
			WSACleanup();
			return -1;
		}
		else if (FD_ISSET(listenSocket, &readfds)) { //zahtev za konekciju
			printf("Client \n");
			acceptedSocket[currentClients] = accept(listenSocket, (SOCKADDR*)&client, &size);

			if (acceptedSocket[currentClients] == INVALID_SOCKET)
			{
				closesocket(acceptedSocket[currentClients]);
				printf("Client connection error %d", currentClients);
				continue;
			}

			DWORD communicaton;//nemam pojma cemu sluzi
			HANDLE threadHandler;
			threadHandler = CreateThread(NULL, 0, &Communication_with_client, &acceptedSocket[currentClients], 0, &communicaton);
			if (threadHandler == NULL)
			{
				printf("Error with creating a thread\n");
			}

			/*iResult = ioctlsocket(acceptedSocket[currentClients], FIONBIO, &mode);

			if (iResult == SOCKET_ERROR)
			{
				printf("blocking error");
				closesocket(acceptedSocket[currentClients]);
				continue;
			}*/

			printf("client %d done \n", currentClients);
			currentClients++;
		}
		//else { //desavanje na klijentu
		//	for (int i = 0; i < currentClients; i++)
		//	{
		//		if (FD_ISSET(acceptedSocket[i], &readfds))
		//		{
		//			iResult = recv(acceptedSocket[i], dataBuffer, BUFFER_SIZE, 0);

		//			if (iResult > 0) { //poruka

		//				Message* msg = (Message*)dataBuffer;
		//				printf(" Message is %s /n", msg->val);


		//			}
		//			else if (iResult == 0)
		//			{
		//				printf("iRes 0 error\n");
		//				closesocket(acceptedSocket[i]);
		//				Compact(acceptedSocket, i, currentClients);
		//			}
		//			else if(iResult == SOCKET_ERROR)
		//			{
		//				printf("Data recv error\n");
		//				closesocket(acceptedSocket[i]);
		//				Compact(acceptedSocket, i, currentClients);
		//			}
		//		}
		//	}
	}
}
	

















  //  do
  //  {
		//// Struct for information about connected client
		//sockaddr_in clientAddr;

		//int clientAddrSize = sizeof(struct sockaddr_in);

  //      // Accept new connections from clients 
  //      acceptedSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &clientAddrSize);

		//// Check if accepted socket is valid 
  //      if (acceptedSocket == INVALID_SOCKET)
  //      {
  //          printf("accept failed with error: %d\n", WSAGetLastError());
  //          closesocket(listenSocket);
  //          WSACleanup();
  //          return 1;
  //      }

		//printf("\nNew client request accepted. Client address: %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		//do
		//{
		//	// Receive data until the client shuts down the connection
		//	iResult = recv(acceptedSocket, dataBuffer, BUFFER_SIZE, 0);
  //      
		//	if (iResult > 0)	// Check if message is successfully received
		//	{
		//		dataBuffer[iResult] = '\0';

		//		// Log message text
		//		printf("Client sent: %s.\n", dataBuffer);

		//	}
		//	else if (iResult == 0)	// Check if shutdown command is received
		//	{
		//		// Connection was closed successfully
		//		printf("Connection with client closed.\n");
		//		closesocket(acceptedSocket);
		//	}
		//	else	// There was an error during recv
		//	{
  //         
		//		printf("recv failed with error: %d\n", WSAGetLastError());
		//		closesocket(acceptedSocket);
		//	}

		//}while(iResult > 0);

  //      // Here is where server shutdown loguc could be placed

  //  } while (true);

    // Shutdown the connection since we're done
 //   iResult = shutdown(acceptedSocket, SD_BOTH);

	//// Check if connection is succesfully shut down.
 //   if (iResult == SOCKET_ERROR)
 //   {
 //       printf("shutdown failed with error: %d\n", WSAGetLastError());
 //       closesocket(acceptedSocket);
 //       WSACleanup();
 //       return 1;
 //   }

 //   //Close listen and accepted sockets
 //   closesocket(listenSocket);
 //   closesocket(acceptedSocket);

	//// Deinitialize WSA library
 //   WSACleanup();

