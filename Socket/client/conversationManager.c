#include "conversationManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <jansson.h>  

#define FILE_PREFIX "conversation_"
#define FILE_SUFFIX ".json"

static char* build_filename(const char* target) {
    static char filename[256];
    snprintf(filename, sizeof(filename), "%s%s%s", FILE_PREFIX, target, FILE_SUFFIX);
    return filename;
}

void create_conversation_file_if_not_exists(const char* target) {
    char* filename = build_filename(target);
    struct stat buffer;
    if (stat(filename, &buffer) != 0) {
        FILE* fp = fopen(filename, "w");
        if (fp) {
            fputs("{\"messages\": []}", fp);
            fclose(fp);
        }
    }
}



void save_message_to_json(const char* sender, const char* target, const char* message) {
    char* filename = build_filename(target);
    create_conversation_file_if_not_exists(target);

    json_t* root;
    json_error_t error;

    FILE* fp = fopen(filename, "r");
    if (!fp) return;

    root = json_loadf(fp, 0, &error);
    fclose(fp);

    if (!root || !json_is_object(root)) {
        json_decref(root);
        return;
    }

    json_t* messages = json_object_get(root, "messages");
    if (!messages || !json_is_array(messages)) {
        json_decref(root);
        return;
    }

    json_t* msg = json_object();
    json_object_set_new(msg, "sender", json_string(sender));
    json_object_set_new(msg, "message", json_string(message));

    json_array_append_new(messages, msg);

    fp = fopen(filename, "w");
    if (!fp) {
        json_decref(root);
        return;
    }

    json_dumpf(root, fp, JSON_INDENT(2));
    fclose(fp);
    json_decref(root);
}
