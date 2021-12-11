#pragma once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_SIZE_NAME 15
#define MAX_MESSAGE_SIZE 256

typedef struct Message {
	char message_content[MAX_MESSAGE_SIZE];
	int size_of_message;
	char destination[MAX_SIZE_NAME];
	char source[MAX_SIZE_NAME];
}Message;

typedef struct QueueNode {
	struct Message message;
	struct QueueNode* next;
}QueueNode;

void Init_Queue(QueueNode **head);
void Enqueue(QueueNode** head, struct Message message);
Message* Dequeue(QueueNode** head);
void PrintQueue(QueueNode* head);
void DeleteQueue(QueueNode** head);