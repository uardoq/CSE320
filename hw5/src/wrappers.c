#include "wrappers.h"

#define BAD_REQ() (response_header_t) {.response_code = BAD_REQUEST, .value_size = 0}
#define MAP_RES(res_code, val_size) (response_header_t) {.response_code = res_code, .value_size = val_size}

void service_request(int connfd, hashmap_t **hashmap) {
    request_header_t req;
    response_header_t res;
    map_key_t key;
    map_val_t val;
    char *key_base, *val_base;

    Rio_readn(connfd, &req, sizeof(request_header_t));

    if (validate_keyval(req.key_size, req.value_size) == false) {
        Rio_writen(connfd, &BAD_REQ(), sizeof(res));
        return;
    }

    // allocate new map_key_t and map_val_t
    if ( (key_base = Calloc(1, req.key_size)) == NULL ) {
        Rio_writen(connfd, &BAD_REQ(), sizeof(res));
        return;
    }
    if ( (val_base = Calloc(1, req.value_size)) == NULL ) {
        Rio_writen(connfd, &BAD_REQ(), sizeof(res));
        return;
    }

    Rio_readn(connfd, key_base, req.key_size);
    Rio_readn(connfd, val_base, req.value_size);

    key = MAP_KEY(key_base, req.key_size);
    val = MAP_VAL(val_base, req.value_size );

    switch (req.request_code) {
    case PUT:
        debug("%s\n", "PUT");
        handle_put_req(connfd, hashmap, key, val);
        break;
    case GET:
        debug("%s\n", "GET");
        handle_get_req(connfd, hashmap, key);
        break;
    case EVICT:
        debug("%s\n", "EVICT");
        handle_evict_req(connfd, hashmap, key);
        break;
    case CLEAR:
        debug("%s\n", "CLEAR");
        handle_clear_req(connfd, hashmap);
        break;
    default:
        debug("%s\n", "Invalid Request");
        Rio_writen(connfd, &MAP_RES(UNSUPPORTED, 0), sizeof(response_header_t));
        break;
    }

}

void handle_clear_req(int connfd, hashmap_t **hashmap) {

    // dont do anything with the return value?
    clear_map(*hashmap);
    Rio_writen(connfd, &MAP_RES(OK, 0), sizeof(response_header_t));
}


void handle_evict_req(int connfd, hashmap_t **hashmap, map_key_t key) {

    // dont do anything with the return node?
    map_node_t retnode = delete(*hashmap, key);
    if (retnode.key.key_base != NULL) {
        (*hashmap)->destroy_function(retnode.key, retnode.val);
    }
    Rio_writen(connfd, &MAP_RES(OK, 0), sizeof(response_header_t));

}

void handle_get_req(int connfd, hashmap_t **hashmap, map_key_t key) {

    map_val_t retval = get(*hashmap, key);
    if ( retval.val_base == NULL && retval.val_len == 0) {
        // could not find the val using key
        Rio_writen(connfd, &MAP_RES(NOT_FOUND, 0), sizeof(response_header_t));
    } else {
        Rio_writen(connfd, &MAP_RES(OK, retval.val_len), sizeof(response_header_t));
        Rio_writen(connfd, retval.val_base, retval.val_len);
    }

}

void handle_put_req(int connfd, hashmap_t **hashmap, map_key_t key, map_val_t val) {

    if (put(*hashmap, key, val, true)  == false) {
        // could not put
        Rio_writen(connfd, &BAD_REQ(), sizeof(response_header_t));
    } else {
        Rio_writen(connfd, &MAP_RES(OK, val.val_len), sizeof(response_header_t));
    }

}

bool validate_keyval(uint32_t key_size, uint32_t val_size) {
    if (key_size == 0 || val_size == 0)
        return true;
    if (key_size >= MIN_KEY_SIZE && key_size <= MAX_KEY_SIZE &&
            val_size >= MIN_VALUE_SIZE && val_size <= MAX_VALUE_SIZE)
        return true;
    return false;
}

int key_cmp(map_key_t key1, map_key_t key2) {
    return (key1.key_len == key2.key_len) ?
           memcmp(key1.key_base, key2.key_base, key1.key_len) : -1;
}

void handle_args(int argc, char *argv[], int *NUM_WORKERS, char **PORT_NUMBER, int *MAX_ENTRIES) {

    if (argc < 2)
        exit(EXIT_FAILURE);

    if (strcmp(argv[1], "-h") == 0) {
        printf("%s\n", "./cream [-h] NUM_WORKERS PORT_NUMBER MAX_ENTRIES");
        printf("%s\n", "-h\t\tDisplays this help menu and returns EXIT_SUCCESS.\nNUM_WORKERS\tThe number of worker threads used to service requests.\nPORT_NUMBER\tPort number to listen on for incoming connections.\nMAX_ENTRIES\tThe maximum number of entries that can be stored in `cream`'s underlying data store.");
        exit(EXIT_SUCCESS);
    }
    else {
        if ((*NUM_WORKERS = atoi(argv[1])) < 1 ) {
            printf("%s\n", "invalid number of workers");
            exit(EXIT_FAILURE);
        }
        if ((*MAX_ENTRIES = atoi(argv[3])) < 1 ) {
            printf("%s\n", "invalid capacity");
            exit(EXIT_FAILURE);
        }
        *PORT_NUMBER = argv[2];
    }
}
