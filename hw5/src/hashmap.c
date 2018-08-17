#include "utils.h"
#include "wrappers.h"
#include <errno.h>
#include "debug.h"
#include "csapp.h"

#define MAP_KEY(base, len) (map_key_t) {.key_base = base, .key_len = len}
#define MAP_VAL(base, len) (map_val_t) {.val_base = base, .val_len = len}
#define MAP_NODE(key_arg, val_arg, tombstone_arg) (map_node_t) {.key = key_arg, .val = val_arg, .tombstone = tombstone_arg}

hashmap_t *create_map(uint32_t capacity, hash_func_f hash_function, destructor_f destroy_function) {
    hashmap_t *hm = NULL;

    if (capacity < 1 || hash_function == NULL || destroy_function == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if ((hm = Calloc(1, sizeof(hashmap_t))) == NULL) {
        return NULL;
    }
    if ((hm->nodes = Calloc(capacity, sizeof(map_node_t))) == NULL) {
        return NULL;
    }
    if (pthread_mutex_init(&hm->write_lock, NULL) == -1) {
        return NULL;
    }
    if (pthread_mutex_init(&hm->fields_lock, NULL) == -1) {
        return NULL;
    }
    hm->capacity = capacity;
    hm->size = 0;
    hm->hash_function = hash_function;
    hm->destroy_function = destroy_function;
    hm->num_readers = 0;
    hm->invalid = false;

    return hm;
}

bool put(hashmap_t *self, map_key_t key, map_val_t val, bool force) {
    /** writer **/

    // only allow the writer to write when read count is 0
    // this assures readers have the priority

    uint32_t index = 0;
    uint32_t count = 0;             // used to count the number of nodes we've traversed
    uint32_t offset = 0;
    map_node_t *pnode = NULL;

    pthread_mutex_lock(&self->write_lock);

    // parameter checks
    if (self == NULL || self->invalid ||
            key.key_base == NULL || key.key_len == 0 ||
            val.val_base == NULL || val.val_len == 0 ) {
        // errno is not shared between threads because it belongs in the stack
        errno = EINVAL;
        return false;
    }

    // basic implementation overwrites if the cache is full
    if (self->size == self->capacity && force == false) {
        errno = ENOMEM;
        return false;
    }

    // calc node index in hash array
    index = get_index(self, key);
    pnode = self->nodes + ((index + offset) % self->capacity);

    /**
     * fields lock here because we read map size so we
     * dont want any writers increasing the size
     */
    // overwrite at hashed index
    if (self->size == self->capacity && force) {
        // pthread_mutex_unlock(&self->fields_lock);
        self->destroy_function(pnode->key, pnode->val);
        // pthread_mutex_lock(&self->write_lock);
        pnode->key = key;
        pnode->val = val;
        pnode->tombstone = false;
        pthread_mutex_unlock(&self->write_lock);
        return true;
    }

    if (pnode->tombstone == true) {
        // if node is tombstone, overwrite it
        self->destroy_function(pnode->key, pnode->val);     // don't know if this is required
        pnode->key = key;
        pnode->val = val;
        pnode->tombstone = false;
        self->size += 1;
        pthread_mutex_unlock(&self->write_lock);
        return true;
    }

    // false tombstone = node is empty or node is used
    // true tombstone = node was deleted
    // node is empty, or node is already used by same key (must update), or collission
    while (count < self->capacity) {
        // loop until we find usable node
        if (pnode->tombstone == false && key_cmp(pnode->key, key) == 0) {
            // update value at key
            pnode->val = val;
            pthread_mutex_unlock(&self->write_lock);
            return true;
        }
        else if (pnode->tombstone == false && pnode->key.key_base == NULL) {
            // node is empty or is a tombstone
            pnode->key = key;
            pnode->val = val;
            pnode->tombstone = false;
            self->size += 1;
            pthread_mutex_unlock(&self->write_lock);
            return true;
        }
        // next node in backing array
        pnode = self->nodes + ((index + offset) % self->capacity);
        offset++;
        count++;
    }

    pthread_mutex_unlock(&self->write_lock);
    return false;
}

map_val_t get(hashmap_t *self, map_key_t key) {

    uint32_t index = 0;
    uint32_t count = 0;
    uint32_t offset = 0;
    map_node_t *node = NULL;
    map_val_t retVal = MAP_VAL(NULL, 0);

    pthread_mutex_lock(&self->fields_lock);
    self->num_readers++;
    if (self->num_readers == 1) {
        // * take the write mutex so the writer doesn't have access to the data store
        // * or wait for the writer to finish writing before accessing the data store
        pthread_mutex_lock(&self->write_lock);
    }
    pthread_mutex_unlock(&self->fields_lock);

    /** critical section */
    if (self == NULL || key.key_base == NULL || key.key_len == 0 || self->invalid) {
        errno = EINVAL;
        retVal = MAP_VAL(NULL, 0);
    }
    else {

        index = get_index(self, key);
        node = self->nodes + ((index + offset) % self->capacity);

        while (count < self->capacity) {
            if (node->tombstone == false && key_cmp(node->key, key) == 0) {
                retVal = node->val;
                break;
            }
            node = self->nodes + index + offset++ % self->capacity;
            count++;
        }
    }
    /** critical section */

    pthread_mutex_lock(&self->fields_lock);
    self->num_readers--;
    if (self->num_readers == 0) {
        pthread_mutex_unlock(&self->write_lock);
    }
    pthread_mutex_unlock(&self->fields_lock);

    return retVal;
}

map_node_t delete(hashmap_t *self, map_key_t key) {

    uint32_t index  = 0, count = 0, offset = 0;
    map_node_t *retnode = NULL;

    pthread_mutex_lock(&self->write_lock);

    if (self == NULL || key.key_base == NULL || key.key_len == 0 || self->invalid) {
        errno = EINVAL;
        // return empty node
        pthread_mutex_unlock(&self->write_lock);
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }

    // delete node
    index = get_index(self, key);
    retnode = self->nodes + index;

    while (count < self->capacity) {
        // loop until we find usable node
        if (retnode->tombstone == false && key_cmp(retnode->key, key) == 0) {
            // when we delete a slot, set the tombstone flag at index
            self->size--;
            retnode->tombstone = true;
            pthread_mutex_unlock(&self->write_lock);
            return *retnode;
        }
        // next node in backing array
        retnode = self->nodes + ((index + offset) % self->capacity);
        offset++;
        count++;
    }

    pthread_mutex_unlock(&self->write_lock);
    return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
}

bool clear_map(hashmap_t *self) {

    pthread_mutex_lock(&self->write_lock);
    if (self == NULL || self->nodes == NULL || self->invalid) {
        errno = EINVAL;
        return false;
    }

    // get base node
    map_node_t *currentNode = self->nodes;
    int offset = 0;

    while (offset < self->capacity) {
        // @1288
        // call destroy_function only on nodes that are not tombstones
        if (currentNode->tombstone == false) {
            currentNode->tombstone = true;
            self->destroy_function(currentNode->key, currentNode->val);
        }


        currentNode = self->nodes + offset;
        offset++;
    }

    self->size = 0;
    // pthread_mutex_unlock(&self->fields_lock);
    pthread_mutex_unlock(&self->write_lock);

    return true;
}

bool invalidate_map(hashmap_t *self) {

    pthread_mutex_lock(&self->write_lock);

    if (self == NULL || self->nodes == NULL || self->invalid) {
        errno = EINVAL;
        return false;
    }

    map_node_t *currentNode = self->nodes;
    int offset = 0;


    while (offset < self->capacity) {
        // call destroy_function on every node
        self->destroy_function(currentNode->key, currentNode->val);
        offset++;
        currentNode = self->nodes + offset;
    }

    free(self->nodes);
    self->invalid = true;

    pthread_mutex_unlock(&self->write_lock);

    return true;
}
