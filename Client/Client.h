#pragma once
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "conio.h"
#include "../Common/Hash_table.h"
SOCKET Connect(char* queueName);
int SendMsg(char* queueName, char* message, int messageSize, SOCKET* connectSocket);