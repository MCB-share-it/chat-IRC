#ifndef CONVERSATION_MANAGER_H
#define CONVERSATION_MANAGER_H

#include <stddef.h>

typedef struct {
    char* sender;
    char* message;
    char* timestamp;
} ChatMessage;

void save_message_to_json(const char* sender, const char* target, const char* message);
void create_conversation_file_if_not_exists(const char* target);

#endif
