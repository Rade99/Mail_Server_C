#include "Hash_table.h"

#define TABLE_SIZE 10
#define MAX_SIZE 256

QueueTable * hash_table[TABLE_SIZE];

unsigned int hash(const char* name)
{
	int lenght = strnlen(name, MAX_SIZE);
	unsigned int hash_value = 0;

	for (int i = 0; i < lenght; i++)
	{
		hash_value += name[i];
		hash_value = (hash_value * name[i]) % TABLE_SIZE;
	}

	return hash_value;
}

void init_hash_table() {
	for (int i = 0; i < TABLE_SIZE; i++) {
		hash_table[i] = NULL;
	}
}

void print_table() {
	for (int i = 0; i < TABLE_SIZE; i++) {
		if (hash_table[i] == NULL)
		{
			printf("%d\t\tUNDEFINED SLOT\n", i);
		}
		else {
			printf("%d\t Hash:%d \t ime: %s\t\n\n", i, hash(hash_table[i]->name), hash_table[i]->name);
			printf("INBOX\n--------------------------------\n");
			PrintQueue(hash_table[i]->value->inbox_start);
			printf("\nOUTBOX\n--------------------------------\n");
			PrintQueue(hash_table[i]->value->outbox_start);
		}
	}

	printf("\n\n");
}

void hash_table_insert_client(const char* key)
{
	QueueTable* p = (QueueTable*)malloc(sizeof(QueueTable));
	if (p == NULL) {
		printf("Not enough RAM!\n");
		exit(21);
	}

	strcpy(p->name, key);

	TableValue *tv = (TableValue*)malloc(sizeof(struct TableValue));
	p->value = tv;

	Init_Queue(&tv->inbox_start);
	Init_Queue(&tv->outbox_start);

	int index = hash(p->name);
	for (int i = 0; i < TABLE_SIZE; i++)
	{
		int safe_index = (i + index) % TABLE_SIZE;
		if (hash_table[safe_index] == NULL) {
			hash_table[safe_index] = p;
			break;
		}
	}
}

TableValue *hash_table_retreive(const char* key)
{
	int index = hash(key);
	for (int i = 0; i < TABLE_SIZE; i++) {
		int safe_index = (i + index) % TABLE_SIZE;
		if (hash_table[safe_index] != NULL && strncmp(hash_table[safe_index]->name, key, TABLE_SIZE) == 0)
			return hash_table[safe_index]->value;
	}

	return NULL;
}

void insert_message_in(const char* key, Message message, const char* in_out)
{
	TableValue *tv = hash_table_retreive(key);
	if (strcmp("in", in_out) == 0)//ako je inbox
	{
		Enqueue(&tv->inbox_start, message);
	}
	else if (strcmp("out", in_out) == 0)
	{
		Enqueue(&tv->outbox_start, message);
	}
	else
	{
		return;
	}
}

void free_hash()//proveri
{
	for (int i = 0; i < TABLE_SIZE; i++) {
		if (hash_table[i] == NULL)
		{
			continue;
		}
		else {// NE ZNAM DA LI JA OVO DOBRO RADIM
			DeleteQueue(&(hash_table[i]->value->inbox_start));
			DeleteQueue(&(hash_table[i]->value->outbox_start));
			free(hash_table[i]->value);
			free(hash_table[i]);
		}
	}
}
