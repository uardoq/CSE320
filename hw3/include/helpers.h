#ifndef HELPERS_H
#define HELPERS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "sfmm.h"

/**
 * Find first available free block of size block_size starting in list index start_at_list_index.
 * searches in next largest list if no free block is found in current list.
 * repeats search until no more lists are availble, i.e. until FREE_LIST_COUNT
 * Returns NULL if free block is not found.
 * @param block_size with initial padding, i.e. word aligned, min 32 bytes
 * @param start_at_list_index starts search in this list, searches forward until FREE_LIST_COUNT
 */
void* find_fit(size_t block_size, int start_at_list_index);

int find_list_index(size_t block_size);

void remove_free_node(sf_free_header *node);

size_t calc_block_size(size_t size);

/**
 * coalesce lower adress (header) with higher address (header + block_size_bytes) and updates
 * the new block's header and footer.
 * @param  header           [description]
 * @param  block_size_bytes [description]
 * @return                  [description]
 */
sf_free_header *coalesce(sf_header *header, size_t block_size_bytes);


/**
 * Inserts free_head at the beginning of free list, updates prev and next ptrs (LIFO)
 * @param free_head  [description]
 * @param list_index [description]
 */
void put_at_head_of_list(sf_free_header* free_head, int list_index);


sf_footer *get_footer_addr(sf_header* header);

sf_free_header *split_if_no_splint(sf_free_header *free_header, size_t block_size_bytes);

void set_tags_for_free_block(sf_header *free_header, size_t block_size_in_bytes);

sf_header *get_header_addr(sf_footer* footer);

void allocate_block(sf_header *header, size_t requested_size);

#endif

