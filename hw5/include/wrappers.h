#ifndef WRAPPERS_H
#define WRAPPERS_H

#include <string.h>
#include <stdio.h>
#include "queue.h"
#include "hashmap.h"
#include "cream.h"
#include "csapp.h"
#include "utils.h"
#include "debug.h"

void handle_args(int argc, char *argv[], int *NUM_WORKERS, char **PORT_NUMBER, int *MAX_ENTRIES);

int key_cmp(map_key_t key1, map_key_t key2);

void service_request(int connfd, hashmap_t **hashmap);

bool validate_keyval(uint32_t key_size, uint32_t val_size);

void handle_put_req(int connfd, hashmap_t **hashmap, map_key_t key, map_val_t val);

void handle_get_req(int connfd, hashmap_t **hashmap, map_key_t key);

void handle_evict_req(int connfd, hashmap_t **hashmap, map_key_t key);

void handle_clear_req(int connfd, hashmap_t **hashmap);

#endif