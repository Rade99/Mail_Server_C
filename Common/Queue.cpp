#define _CRT_SECURE_NO_WARNINGS
#include "Queue.h"

void Init_Queue(QueueNode **head)
{
	*head = NULL;
}

void Enqueue(QueueNode** head, struct Message message)
{
	QueueNode* newNode = (QueueNode*)malloc(sizeof(struct QueueNode)); // napravim novi cvor tjst pokazivac na QUEUE, da ga popunim i da pronadjem mesto u redu

	if (newNode == NULL) {
		printf("Not enough RAM!\n");
		exit(21);
	}

	strcpy(newNode->message.destination, message.destination);
	strcpy(newNode->message.message_content, message.message_content);
	newNode->message.size_of_message = message.size_of_message; // koja je fora sa ovim poljem

	newNode->next = NULL;

	if (*head == NULL) { // list is empty
		*head = newNode;
		return;
	}
	else
	{
		QueueNode* pomocni = *head;// da ne bi pomerali head
		while (pomocni->next != NULL) {
			pomocni = pomocni->next;//traverse the list until p is the last node.The last node always points to NULL.
		}
		pomocni->next = newNode;//Point the previous last node to the new node created. //dodaje na kraj reda-liste
	}
}

Message* Dequeue(QueueNode** head)// nije do kraja
{
	QueueNode* p = *head;
	if (p == NULL)
	{
		return NULL;//red je prazan nije moguc dequeue
	}

	Message* ret = (Message*)malloc(sizeof(struct Message));
	if (ret == NULL) {
		printf("Not enough RAM!\n");
		exit(21);
	}


	strcpy(ret->destination, p->message.destination);
	strcpy(ret->message_content, p->message.message_content);
	ret->size_of_message = p->message.size_of_message;

	*head = p->next;// glava je sada sledeci
	free(p);// oslobadjamo memoriju prvog kog smo ocitali

	return ret;
}

void PrintQueue(QueueNode* head)
{
	//po,mocna promenljiva bi nam trebala samo ako se radi o referenci jer bi se glaa meenjala pa bi morali to negde da zapamtimo
	while (head != NULL)// msms da moze i sa next ejr na kraju next pokazuje na kraj liste
	{
		printf("poruka: %s, odrediste %s, velicina %d\n", head->message.message_content, head->message.destination, head->message.size_of_message);//videcemo sta
		head = head->next;// nisu poredjane jedno za drugim
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