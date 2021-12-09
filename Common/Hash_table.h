#pragma once
#include "Queue.h"

#define TABLE_SIZE 10


typedef struct QueueTable {
	char name[MAX_SIZE_NAME]; //key
	struct TableValue* value;

}QueueTable;

typedef struct TableValue {
	struct QueueNode* inbox_start;
	struct QueueNode* outbox_start;
}TableValue;

unsigned int hash(const char* name);
void init_hash_table();
void print_table();
void hash_table_insert_client(const char* key);
TableValue *hash_table_retreive(const char* key);
void insert_message_in(const char* key, Message message, const char* in_out);
void free_hash();
