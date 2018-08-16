/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "sfmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <string.h>
#include "helpers.h"
/**
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in sfmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
free_list seg_free_list[4] = {
    {NULL, LIST_1_MIN, LIST_1_MAX},
    {NULL, LIST_2_MIN, LIST_2_MAX},
    {NULL, LIST_3_MIN, LIST_3_MAX},
    {NULL, LIST_4_MIN, LIST_4_MAX}
};

int sf_errno = 0;

void *sf_malloc(size_t size) {
    // WORD ALIGNED = 16 bits + 16 bits = 32 bits is the smallest supported block

    size_t block_size, remaining_free_block;
    void *block_ptr                   = NULL;
    sf_header *header                 = NULL;
    sf_free_header *free_space_header = NULL;

    block_size = calc_block_size(size);

    // validate size request
    if (size <= 0 ) {
        sf_errno = EINVAL;
        return NULL;
    }
    if (size >= 4 * PAGE_SZ) {
        sf_errno = ENOMEM;
        return NULL;
    }
    block_ptr = find_fit(block_size, find_list_index(block_size));

    // CASE: List is empty because this is the first malloc.
    // request vm, split, put remaining block into free list.
    if (get_heap_start() == 0) {
        // request as much vm as needed
        int i = 0;
        while ( block_size > i++ * PAGE_SZ) {
            // total allocated heap cannot be greater than 4 pages, only manage up to 4 pages.
            if ( (block_ptr = sf_sbrk()) < 0 /*|| (get_heap_end() - get_heap_start()) > 4 * PAGE_SZ*/) {
                // not enough memory
                sf_errno = ENOMEM;
                return NULL;
            }
            // coalesce free pages
            set_tags_for_free_block(get_heap_start(), i * PAGE_SZ);
        }
        block_ptr = get_heap_start();
        free_space_header = split_if_no_splint(block_ptr, block_size);
        if (block_ptr != free_space_header) {
            // no splint put free block in list
            put_at_head_of_list(free_space_header, find_list_index(free_space_header->header.block_size << 4));
        }

        // allocate block
        allocate_block((sf_header*)block_ptr, size);
        // TEMPORARY
        header = block_ptr;
    }
    // CASE: List contains free blocks but there is no block big enough for the request.
    // request vm, attempt to coalesce with previous heap break.
    else if (block_ptr == NULL && get_heap_start()) {
        int i = 0;
        sf_footer *prev_block_footer = (sf_footer*)get_heap_end() - 1;
        sf_header *prev_block_header = get_header_addr(prev_block_footer);
        size_t prev_block_size       = (prev_block_header->block_size << 4);

        // if the previous block is outside of heap range, throw error
        if ((void*)prev_block_footer < get_heap_start()) {
            // invalid
            sf_errno = ENOMEM;
            return NULL;
        }
        // if prev block is free, update the prev block size (coalesce if possible) and put in list.
        if ( prev_block_footer->allocated == 0 ) {

            remove_free_node((sf_free_header*)prev_block_header);

            while (block_size > i * PAGE_SZ + prev_block_size) {
                if ( (block_ptr = sf_sbrk()) < 0 /*|| (get_heap_end() - get_heap_start()) > 4 * PAGE_SZ*/) {
                    // not enough memory
                    sf_errno = ENOMEM;
                    return NULL;
                }
                i++;
            }
            // coalesce backwards with free (chunk) block
            set_tags_for_free_block(prev_block_header, i * PAGE_SZ + prev_block_size);

            free_space_header = split_if_no_splint((sf_free_header*)prev_block_header, block_size);

            allocate_block((sf_header*)prev_block_header, size);

            // put in free list
            put_at_head_of_list(free_space_header, find_list_index(free_space_header->header.block_size << 4));

            header = prev_block_header;
        }
        else {
            header = get_heap_end();

            // previous block is allocated
            while (block_size > i * PAGE_SZ) {
                if ( (block_ptr = sf_sbrk()) < 0 ) {
                    // not enough memory
                    sf_errno = ENOMEM;
                    return NULL;
                }
                i++;
            }
            // new space ready for splitting
            set_tags_for_free_block(header, i * PAGE_SZ);

            free_space_header = split_if_no_splint((sf_free_header*)header, block_size);

            allocate_block((sf_header*)header, size);

            // put free space in list
            put_at_head_of_list(free_space_header, find_list_index(free_space_header->header.block_size << 4));
        }
    }
    // CASE: block is found.
    // remove free block from list, set header properties.
    else {
        // remove from free list
        remove_free_node((sf_free_header*)block_ptr);

        header = (sf_header*)block_ptr;
        remaining_free_block = (header->block_size << 4) - block_size;

        // free_block size - block_size < 32 then its a splinter
        if ( !(remaining_free_block < 32) ) {
            // split block
            free_space_header = split_if_no_splint((sf_free_header*)header, block_size);
            // add to list
            put_at_head_of_list(free_space_header, find_list_index(remaining_free_block));
        }

        allocate_block(header, size);

    }
    // return the ptr to the beginning of the allocated bit
    return header + 1;
}

void *sf_realloc(void *ptr, size_t size) {

    sf_header *header = (sf_header*)ptr - 1;
    sf_footer *footer = get_footer_addr(header);

    // validate pointer
    // case pointer is NULL
    if ( ptr == NULL ) {
        sf_errno = EINVAL;
        return NULL;
    }
    // case header of block is before heap_start or block ends after heap_end
    if ( (void*)header < get_heap_start() || (void*)footer + 1 > get_heap_end() ) {
        sf_errno = EINVAL;
        return NULL;
    }
    // alloc bit of header or footer is 0 (free)
    if ( header->allocated == 0 || footer->allocated == 0 ) {
        sf_errno = EINVAL;
        return NULL;
    }
    // the requested size, block size do not make sense
    if ( footer->requested_size >= (footer->block_size << 4) ) {
        sf_errno = EINVAL;
        return NULL;
    }
    // the padded and alloc bits in the header and footer are inconsistent
    if ( header->allocated != footer->allocated || header->padded != footer->padded ) {
        sf_errno = EINVAL;
        return NULL;
    }
    // padded bits do not make sense
    if ( ((footer->requested_size + 16 != (footer->block_size << 4)) != header->padded) ) {
        // pad bit should be set, but it is not
        sf_errno = EINVAL;
        return NULL;
    }

    if (ptr && size == 0) {
        sf_free(ptr);
        return NULL;
    }

    // CASE: new request size equal to old size
    if ( footer->requested_size == size ) {
        return ptr;
    }

    // CASE: Going to larger request size
    if ( footer->requested_size < size ) {
        // request larger block using sf_malloc
        void* new_payload = sf_malloc(size);
        if (new_payload == NULL) return NULL;
        sf_header *new_header = (sf_header*)new_payload - 1;
        // copy payload of old block into new block
        memcpy(new_payload, ptr, header->block_size << 4);
        // TODO: ZERO out the padding
        // free up the contents of the old block
        sf_free(ptr);
        // return the new block
        return new_header + 1;
    }

    // CASE: Going to smaller request size
    else {
        if (footer->requested_size - size < 32) {
            // splinter
            header->padded = footer->padded = (header->block_size << 4) != size + 16;
            footer->requested_size = size;
            return ptr;
        }
        else {
            // no splinter split
            // below works in case of no splitting maybe
            // free the block temporarily, new allocation will be put at ptr

            sf_free(ptr);

            void* new_payload = sf_malloc(size);
            if (new_payload == NULL) return NULL;
            // sf_header *new_header = (sf_header*)new_payload - 1;

            return ptr;

        }

    }


    // // split creates splinter




    return NULL;
}

void sf_free(void *ptr) {

    sf_header *header                = (sf_header*)ptr - 1;
    sf_footer *footer                = get_footer_addr(header);
    sf_free_header *free_header_next = (sf_free_header*)(footer + 1);
    size_t new_block_size            = header->block_size << 4;

    // invalid cases
    // case pointer is NULL
    if ( ptr == NULL ) {
        abort();
    }
    // case header of block is before heap_start or block ends after heap_end
    if ( (void*)header < get_heap_start() || (void*)footer + 1 > get_heap_end() ) {
        abort();
    }
    // alloc bit of header or footer is 0 (free)
    if ( header->allocated == 0 || footer->allocated == 0 ) {
        abort();
    }
    // the requested size, block size do not make sense
    if ( footer->requested_size >= (footer->block_size << 4) ) {
        abort();
    }
    // the padded and alloc bits in the header and footer are inconsistent
    if ( header->allocated != footer->allocated || header->padded != footer->padded ) {
        abort();
    }
    // padded bits do not make sense
    if ( ((footer->requested_size + 16 != (footer->block_size << 4)) != header->padded) ) {
        // pad bit should be set, but it is not
        abort();
    }

    // if ( ptr.next ) block can be coalesced, remove from list and coalesce with ptr
    if ( free_header_next->header.allocated == 0 ) {
        // calc new block size
        new_block_size = (header->block_size << 4) + (free_header_next->header.block_size << 4);
        // remove free node from list
        remove_free_node(free_header_next);
        // calc new footer offset
        footer = (sf_footer*)header + (new_block_size / 8) - 1;
    }

    // update headers
    header->unused         = 0;
    footer->allocated      = header->allocated   = 0;
    footer->padded         = header->padded      = 0;
    footer->two_zeroes     = header->two_zeroes  = 0;
    footer->block_size     = header->block_size  = new_block_size >> 4;
    footer->requested_size = 0;

    // put at head of list
    put_at_head_of_list((sf_free_header*)header, find_list_index(new_block_size));

    return;
}
