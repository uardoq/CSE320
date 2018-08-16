#include <criterion/criterion.h>
#include <errno.h>
#include <signal.h>
#include "sfmm.h"
#include "helpers.h"

int find_list_index_from_size(int sz) {
	if (sz >= LIST_1_MIN && sz <= LIST_1_MAX) return 0;
	else if (sz >= LIST_2_MIN && sz <= LIST_2_MAX) return 1;
	else if (sz >= LIST_3_MIN && sz <= LIST_3_MAX) return 2;
	else return 3;
}

Test(sf_memsuite_student, Malloc_an_Integer_check_freelist, .init = sf_mem_init, .fini = sf_mem_fini) {
	sf_errno = 0;
	int *x = sf_malloc(sizeof(int));

	cr_assert_not_null(x);

	*x = 4;

	cr_assert(*x == 4, "sf_malloc failed to give proper space for an int!");

	sf_header *header = (sf_header*)((char*)x - 8);

	/* There should be one block of size 4064 in list 3 */
	free_list *fl = &seg_free_list[find_list_index_from_size(PAGE_SZ - (header->block_size << 4))];

	cr_assert_not_null(fl, "Free list is null");

	cr_assert_not_null(fl->head, "No block in expected free list!");
	cr_assert_null(fl->head->next, "Found more blocks than expected!");
	cr_assert(fl->head->header.block_size << 4 == 4064);
	cr_assert(fl->head->header.allocated == 0);
	cr_assert(sf_errno == 0, "sf_errno is not zero!");
	cr_assert(get_heap_start() + PAGE_SZ == get_heap_end(), "Allocated more than necessary!");
}

Test(sf_memsuite_student, Malloc_over_four_pages, .init = sf_mem_init, .fini = sf_mem_fini) {
	sf_errno = 0;
	void *x = sf_malloc(PAGE_SZ << 2);

	cr_assert_null(x, "x is not NULL!");
	cr_assert(sf_errno == ENOMEM, "sf_errno is not ENOMEM!");
}

Test(sf_memsuite_student, free_double_free, .init = sf_mem_init, .fini = sf_mem_fini, .signal = SIGABRT) {
	sf_errno = 0;
	void *x = sf_malloc(sizeof(int));
	sf_free(x);
	sf_free(x);
}

Test(sf_memsuite_student, free_no_coalesce, .init = sf_mem_init, .fini = sf_mem_fini) {
	sf_errno = 0;
	/* void *x = */ sf_malloc(sizeof(long));
	void *y = sf_malloc(sizeof(double) * 10);
	/* void *z = */ sf_malloc(sizeof(char));

	sf_free(y);

	free_list *fl = &seg_free_list[find_list_index_from_size(96)];

	cr_assert_not_null(fl->head, "No block in expected free list");
	cr_assert_null(fl->head->next, "Found more blocks than expected!");
	cr_assert(fl->head->header.block_size << 4 == 96);
	cr_assert(fl->head->header.allocated == 0);
	cr_assert(sf_errno == 0, "sf_errno is not zero!");
}

Test(sf_memsuite_student, free_coalesce, .init = sf_mem_init, .fini = sf_mem_fini) {
	sf_errno = 0;
	/* void *w = */ sf_malloc(sizeof(long));
	void *x = sf_malloc(sizeof(double) * 11);
	void *y = sf_malloc(sizeof(char));
	/* void *z = */ sf_malloc(sizeof(int));

	sf_free(y);
	sf_free(x);

	free_list *fl_y = &seg_free_list[find_list_index_from_size(32)];
	free_list *fl_x = &seg_free_list[find_list_index_from_size(144)];

	cr_assert_null(fl_y->head, "Unexpected block in list!");
	cr_assert_not_null(fl_x->head, "No block in expected free list");
	cr_assert_null(fl_x->head->next, "Found more blocks than expected!");
	cr_assert(fl_x->head->header.block_size << 4 == 144);
	cr_assert(fl_x->head->header.allocated == 0);
	cr_assert(sf_errno == 0, "sf_errno is not zero!");
}

Test(sf_memsuite_student, freelist, .init = sf_mem_init, .fini = sf_mem_fini) {
	/* void *u = */ sf_malloc(1);          //32
	void *v = sf_malloc(LIST_1_MIN); //48
	void *w = sf_malloc(LIST_2_MIN); //160
	void *x = sf_malloc(LIST_3_MIN); //544
	void *y = sf_malloc(LIST_4_MIN); //2080
	/* void *z = */ sf_malloc(1); // 32

	int allocated_block_size[4] = {48, 160, 544, 2080};

	sf_free(v);
	sf_free(w);
	sf_free(x);
	sf_free(y);

	// First block in each list should be the most recently freed block
	for (int i = 0; i < FREE_LIST_COUNT; i++) {
		sf_free_header *fh = (sf_free_header *)(seg_free_list[i].head);
		cr_assert_not_null(fh, "list %d is NULL!", i);
		cr_assert(fh->header.block_size << 4 == allocated_block_size[i], "Unexpected free block size!");
		cr_assert(fh->header.allocated == 0, "Allocated bit is set!");
	}

	// There should be one free block in each list, 2 blocks in list 3 of size 544 and 1232
	for (int i = 0; i < FREE_LIST_COUNT; i++) {
		sf_free_header *fh = (sf_free_header *)(seg_free_list[i].head);
		if (i != 2)
			cr_assert_null(fh->next, "More than 1 block in freelist [%d]!", i);
		else {
			cr_assert_not_null(fh->next, "Less than 2 blocks in freelist [%d]!", i);
			cr_assert_null(fh->next->next, "More than 2 blocks in freelist [%d]!", i);
		}
	}
}

Test(sf_memsuite_student, realloc_larger_block, .init = sf_mem_init, .fini = sf_mem_fini) {
	void *x = sf_malloc(sizeof(int));
	/* void *y = */ sf_malloc(10);
	x = sf_realloc(x, sizeof(int) * 10);

	free_list *fl = &seg_free_list[find_list_index_from_size(32)];

	cr_assert_not_null(x, "x is NULL!");
	cr_assert_not_null(fl->head, "No block in expected free list!");
	cr_assert(fl->head->header.block_size << 4 == 32, "Free Block size not what was expected!");

	sf_header *header = (sf_header*)((char*)x - 8);
	cr_assert(header->block_size << 4 == 64, "Realloc'ed block size not what was expected!");
	cr_assert(header->allocated == 1, "Allocated bit is not set!");
}

Test(sf_memsuite_student, realloc_smaller_block_splinter, .init = sf_mem_init, .fini = sf_mem_fini) {
	void *x = sf_malloc(sizeof(int) * 8);
	void *y = sf_realloc(x, sizeof(char));

	cr_assert_not_null(y, "y is NULL!");
	cr_assert(x == y, "Payload addresses are different!");

	sf_header *header = (sf_header*)((char*)y - 8);
	cr_assert(header->allocated == 1, "Allocated bit is not set!");
	cr_assert(header->block_size << 4 == 48, "Block size not what was expected!");

	free_list *fl = &seg_free_list[find_list_index_from_size(4048)];

	// There should be only one free block of size 4048 in list 3
	cr_assert_not_null(fl->head, "No block in expected free list!");
	cr_assert(fl->head->header.allocated == 0, "Allocated bit is set!");
	cr_assert(fl->head->header.block_size << 4 == 4048, "Free block size not what was expected!");
}

Test(sf_memsuite_student, realloc_smaller_block_free_block, .init = sf_mem_init, .fini = sf_mem_fini) {
	void *x = sf_malloc(sizeof(double) * 8);
	void *y = sf_realloc(x, sizeof(int));

	cr_assert_not_null(y, "y is NULL!");

	sf_header *header = (sf_header*)((char*)y - 8);
	cr_assert(header->block_size << 4 == 32, "Realloc'ed block size not what was expected!");
	cr_assert(header->allocated == 1, "Allocated bit is not set!");


	// After realloc'ing x, we can return a block of size 48 to the freelist.
	// This block will coalesce with the block of size 4016.
	free_list *fl = &seg_free_list[find_list_index_from_size(4064)];

	cr_assert_not_null(fl->head, "No block in expected free list!");
	cr_assert(fl->head->header.allocated == 0, "Allocated bit is set!");
	cr_assert(fl->head->header.block_size << 4 == 4064, "Free block size not what was expected!");
}


//############################################
//STUDENT UNIT TESTS SHOULD BE WRITTEN BELOW
//DO NOT DELETE THESE COMMENTS
//############################################


Test(sf_memsuite_student, header_props_allocation, .init = sf_mem_init, .fini = sf_mem_fini) {

	// 48 padded block
	sf_header *header = sf_sbrk();

	allocate_block(header, 47);

	cr_assert(0x0000000000000043 ==
	          (header->allocated | (header->padded << 1) | (header->block_size << 4) | header->two_zeroes | header->unused),
	          "header properties not set correctly");
}

Test(sf_memsuite_student, request_larger_than_PAGE_SZ_on_sec_malloc, .init = sf_mem_init, .fini = sf_mem_fini) {

	// a req of PAGE_SZ is followed by another malloc req
	double *ptr0 = (double*)sf_malloc(23);		// becomes a 48 byte block, free block = 4096 - 48 = 4048
	double *ptr1 = (double*)sf_malloc(PAGE_SZ); // becomes a 4112 byte block, free block = 4048 + 4096 = 8144

	if (ptr0 == ptr1){}
	// total requested vm = 8192
	sf_free(ptr0);
	sf_free_header* sfh = seg_free_list[0].head;
	sf_free(ptr1);
	cr_assert( (sfh->header.block_size << 4) == 48,
	           "incorrect free block remaining in list 1, CASE 2, have %d", (sfh->header.block_size << 4) );
}

Test(sf_memsuite_student, request_larger_than_PAGE_SZ, .init = sf_mem_init, .fini = sf_mem_fini) {
	// case: no more vm available for allocation.
	// solution: request vm and allocate block,
	// split the remaining block and put in seg free list[i]

	// a req of PAGE_SZ is the first malloc req
	double *ptr0 = (double*)sf_malloc(PAGE_SZ); // becomes a 4112 byte block
	double *ptr1 = (double*)sf_malloc(23);		// becomes a 48 byte block
	sf_free_header *sfh = seg_free_list[3].head;
	// total requested vm = 8192
	sf_free(ptr0);
	sf_free(ptr1);
	cr_assert( (sfh->header.block_size << 4 == 4032), "incorrect free block remaining in list 3, CASE 1" );
}

Test(sf_memsuite_student, request_equal_size_of_mult_PAGE_SZ, .init = sf_mem_init, .fini = sf_mem_fini) {
	sf_free_header *free_node = NULL;
	size_t free_space         = 0;
	sf_free_header *list_head = NULL;

	double *ptr0 = (double*)sf_malloc(PAGE_SZ - 16);    	// becomes a PAGE_SZ free block = none
	sf_free(ptr0);

	// add up all the freed up space in all lists.
	for (int i = 0; i < FREE_LIST_COUNT; ++i) {
		list_head = seg_free_list[i].head;
		if ( list_head != NULL ) {
			free_space += list_head->header.block_size << 4;
			free_node = list_head->next;

			while ( free_node ) {
				free_space += (free_node->header.block_size) << 4;
				free_node = seg_free_list[i].head->next;
			}
		}
	}
	cr_assert( free_space == PAGE_SZ, "freed up space is not %d", PAGE_SZ);
}

Test(sf_memsuite_student, contents_of_realloced_block_consistent, .init = sf_mem_init, .fini = sf_mem_fini) {

	double val1 = 0;
	double val2 = 0;

	double *ptr0 = (double*)sf_malloc(32);
	val1 = *ptr0 = 20;

	ptr0 = (double*)sf_realloc(ptr0, 64);
	val2 = *ptr0;

	cr_assert(val1 == val2, "realloc contents not consistent, val1 %d val2 %d", val1, val2);

}
