#include "helpers.h"
#include "sfmm.h"

#if DEBUG
#include <stdio.h>
#endif

void* find_fit(size_t block_size, int start_at_list_index) {
    int i = start_at_list_index;
    // char minimum_block_size = 32;

    sf_free_header *current_free_node = seg_free_list[i].head;

    while (i < FREE_LIST_COUNT) {
        // if list is empty check next largest list
        if ( seg_free_list[i].head == NULL ) i++;
        else {
            current_free_node = seg_free_list[i++].head;
            while ( current_free_node != NULL) {
                if ((current_free_node->header.block_size << 4)/* - 16*/ >= block_size)
                    return current_free_node;
                current_free_node = current_free_node->next;
            }
        }
    }
    return NULL;
}

int find_list_index(size_t block_size) {
    for (int i = 0; i < FREE_LIST_COUNT; ++i) {
        if (block_size >= seg_free_list[i].min && block_size <= seg_free_list[i].max)
            return i;
    }
    return FREE_LIST_COUNT - 1;
}


void remove_free_node(sf_free_header *node) {

    // TODO: test if this works, but need more than 1 free node in the list to test.
    if ( node->prev != NULL) {
        node->prev->next = node->next;
    }
    if ( node->next != NULL) {
        node->next->prev = node->prev;
    }

    // if node to be removed is the head of the list
    if (node == seg_free_list[find_list_index(node->header.block_size << 4)].head) {
        seg_free_list[find_list_index(node->header.block_size << 4)].head = NULL;
    }

}

size_t calc_block_size(size_t size) {
    size_t block_size;

    // calculate required block size (without padding)
    block_size = (SF_HEADER_SIZE + SF_FOOTER_SIZE) / 8 + size;
    // add necessary padding
    if (block_size % 16 != 0) {
        int multiple = -1, i = 0;
        // find next multiple of word = 16
        while (block_size > (multiple = 16 * i++));
        block_size = multiple;
    }
    return block_size;
}

void put_at_head_of_list(sf_free_header* free_head, int list_index) {

    if ( seg_free_list[list_index].head == NULL) {
        seg_free_list[list_index].head = free_head;
    }
    else {
        // set previous head as second node
        free_head->prev = NULL;
        free_head->next = seg_free_list[list_index].head;

        // point old head to new head
        seg_free_list[list_index].head->prev = free_head;
        // set new head of list
        seg_free_list[list_index].head = free_head;
    }
}

sf_footer *get_footer_addr(sf_header* header) {
    return (sf_footer*)header + ((header->block_size << 4) / 8) - 1;
}

sf_header *get_header_addr(sf_footer* footer) {
    return (sf_header*)footer - ((footer->block_size << 4) / 8) + 1;
}

sf_free_header *split_if_no_splint(sf_free_header *free_header, size_t split_offset) {
    sf_free_header *new_free_header = NULL;
    // sf_footer *new_free_footer      = NULL;

    size_t free_block_size = (free_header->header.block_size << 4) - split_offset;

    // if splitting causes a splinter return the original free_header block
    if ( free_block_size < 32) {
        return free_header;
    }
    // else split and return the header for the new free block
    new_free_header = (sf_free_header*)((sf_header*)free_header + (split_offset / 8));

    set_tags_for_free_block((sf_header*)new_free_header, free_block_size);

    return new_free_header;
}

void set_tags_for_free_block(sf_header *free_header, size_t block_size_in_bytes) {

    free_header->block_size     = block_size_in_bytes >> 4;
    sf_footer *free_footer      = get_footer_addr(free_header);
    free_footer->allocated      = free_header->allocated        = 0;
    free_footer->padded         = free_header->padded           = 0;
    free_footer->two_zeroes     = free_header->two_zeroes       = 0;
    free_footer->block_size     = free_header->block_size;
    free_header->unused         = 0;
    free_footer->requested_size = 0;
}

void allocate_block(sf_header *header, size_t requested_size) {

    header->block_size     = calc_block_size(requested_size) >> 4;
    header->unused         = 0;
    sf_footer *footer      = get_footer_addr(header);
    footer->allocated      = header->allocated  = 1;
    footer->padded         = header->padded     = (header->block_size << 4) != requested_size + 16;
    footer->two_zeroes     = header->two_zeroes = 0;
    footer->block_size     = header->block_size;
    footer->requested_size = requested_size;

}