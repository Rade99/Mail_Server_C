#define _CRT_SECURE_NO_WARNINGS
#include "Queue.h"

void Init_Queue(QueueNode **head)
{
	*head = NULL;
}

void Enqueue(QueueNode** head, struct Message message)
{
	QueueNode* newNode = (QueueNode*)malloc(sizeof(struct QueueNode)); 

	if (newNode == NULL) {
		printf("Not enough RAM!\n");
		exit(21);
	}

	strcpy(newNode->message.destination, message.destination);
	strcpy(newNode->message.message_content, message.message_content);
	newNode->message.size_of_message = message.size_of_message; 
	strcpy(newNode->message.source, message.source);

	newNode->next = NULL;

	if (*head == NULL) { 
		*head = newNode;
		return;
	}
	else
	{
		QueueNode* pomocni = *head;
		while (pomocni->next != NULL) {
			pomocni = pomocni->next;
		}
		pomocni->next = newNode;
	}
}

Message* Dequeue(QueueNode** head)
{
	QueueNode* p = *head;
	if (p == NULL)
	{
		return NULL;
	}

	Message* ret = (Message*)malloc(sizeof(struct Message));
	if (ret == NULL) {
		printf("Not enough RAM!\n");
		exit(21);
	}


	strcpy(ret->destination, p->message.destination);
	strcpy(ret->message_content, p->message.message_content);
	strcpy(ret->source, p->message.source);
	ret->size_of_message = p->message.size_of_message;

	*head = p->next;
	free(p);

	return ret;
}

void PrintQueue(QueueNode* head)
{
	
	while (head != NULL)
	{
		printf("poruka: %s, odrediste %s, velicina %d\n", head->message.message_content, head->message.destination, head->message.size_of_message);
		head = head->next;
	}
}

void DeleteQueue(QueueNode** head)
{
	QueueNode* p;
	while (*head != NULL)
	{
		p = *head;
		*head = (*head)->next;
		free(p);
	}
}