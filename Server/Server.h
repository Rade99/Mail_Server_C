#pragma once
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "conio.h"
#include "../Common/Hash_table.h"
DWORD WINAPI Communication_with_client(LPVOID lpParam);
