#pragma once
SOCKET Connect(char* queueName);
void SendMsg(char* queueName, void* message, int messageSize);