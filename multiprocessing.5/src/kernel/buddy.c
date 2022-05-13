#include "buddy.h"

#define PHYS_MEM_ORDER 29 /* 512 MB */
#define PHYS_MEM_SIZE (1UL << PHYS_MEM_ORDER)

#define PHYS_MEM_PAGE_ORDER 12
#define PHYS_MEM_PAGE_SIZE (1UL << PHYS_MEM_PAGE_ORDER)

#define PHYS_MEM_START 0x80000000
#define PHYS_MEM_END (PHYS_MEM_START + PHYS_MEM_SIZE)
#define PHYS_MEM_KERNEL_STACK_SIZE (10 * PHYS_MEM_PAGE_SIZE)

/* Buddy-specific macros */
#define BUDDY_MAX_ORDER (PHYS_MEM_ORDER - PHYS_MEM_PAGE_ORDER)

/* | 0x80000000                        |                        |             |
 * | .text segments... .text of buddy  | buddy's .data and .bss | -->         |
 * |                                   |                     <-- kernel stack |
 */

/* __ppn is buddy's internal ppn, with __ppn == 0 for PHYS_MEM_START */
#ifndef phys2ppn
#define phys2ppn(phys) ((phys) >> PHYS_MEM_PAGE_ORDER)
#endif

#define phys2__ppn(phys) (((phys)-PHYS_MEM_START) >> PHYS_MEM_PAGE_ORDER)

#define __ppn2ppn(__ppn) ((__ppn) + phys2ppn(PHYS_MEM_START))
#define ppn2__ppn(ppn) ((ppn)-phys2ppn(PHYS_MEM_START))

#define __ppn2block(__ppn) ((__ppn) + buddy_blocks)
#define block2__ppn(block) ((block)-buddy_blocks)

#define phys2block(phys) __ppn2block(phys2__ppn(phys))

#define block2buddy(block, order)                                              \
	__ppn2block(block2__ppn(block) ^ (1UL << order))

#define is_list_empty(order) (!(uintptr_t)buddy_free_lists[order])
#define is_block_free(block) (!(block->refcnt))

static struct block *buddy_free_lists[BUDDY_MAX_ORDER + 1];
static struct block buddy_blocks[1UL << BUDDY_MAX_ORDER];

struct buddy_layout layout;

extern uintptr_t phys_base;

static struct block *buddy_pop(unsigned order)
{
	if (is_list_empty(order)) return NULL;

	struct block *block = buddy_free_lists[order];

	/* No need to touch block->order here */
	block->refcnt = 1;
	buddy_free_lists[order] = block->next;
	block->next = NULL;

	return block;
}

static struct block *buddy_remove(struct block *block, unsigned order)
{
	if (is_list_empty(order)) return NULL;

	if (buddy_free_lists[order] != block) {
		struct block *head = buddy_free_lists[order];
		struct block *tail = head;
		struct block *prev = NULL;

		while (tail->next) {
			if (tail->next == block) prev = tail;
			tail = tail->next;
		}

		if (!prev) return NULL; /* Could not find block */

		buddy_free_lists[order] = block;
		prev->next = NULL;
		tail->next = head;
	}

	return buddy_pop(order);
}

static void buddy_push(struct block *block, unsigned order)
{
	block->order = order;
	block->refcnt = 0;
	block->next = buddy_free_lists[order];
	buddy_free_lists[order] = block;
}

static int __buddy_find_smallest_free_order(unsigned order)
{
	for (unsigned i = order; i <= BUDDY_MAX_ORDER; i++) {
		if (!is_list_empty(i)) return i;
	}
	return -1;
}

static int __buddy_split(unsigned order)
{
	if (order == 0) return -1;

	struct block *block = buddy_pop(order);

	if (!block) return -2;

	block->order--; /* Where the magic happens */

	buddy_push(block2buddy(block, block->order), block->order);
	buddy_push(block, block->order);

	return 0;
}

static int buddy_split(unsigned smallest_free_order, unsigned desired_order)
{
	/* Will do nothing if smallest_free and desired are equal */
	for (unsigned i = smallest_free_order; i > desired_order; i--) {
		int ret = __buddy_split(i);
		if (ret) return ret;
	}
	return 0;
}

static struct block *__buddy_merge(struct block *block, struct block *buddy)
{
	/* 
	 * Make sure block is below buddy, as we'll be increasing the order of
	 * block
	 */
	if (block > buddy) return __buddy_merge(buddy, block);

	/*
	 * Need to remove both because we're increasing the order, hence block
	 * is pushed onto a different free list
	 */
	buddy_remove(block, block->order);
	buddy_remove(buddy, buddy->order);

	block->order++; /* Order of buddy never changes */

	buddy_push(block, block->order);

	return block;
}

static void __buddy_try_merge(struct block *block)
{
	if (block->order == BUDDY_MAX_ORDER) return;

	struct block *buddy = block2buddy(block, block->order);

	if (block->order != buddy->order) return;

	if (is_block_free(block) && is_block_free(buddy)) {
		block = __buddy_merge(block, buddy);
		__buddy_try_merge(block);
	}
}

int buddy_free(struct block *block)
{
	switch (block->refcnt) {
	case 0:
		return -1; /* Double free */
	case 1:
		buddy_push(block, block->order);
		__buddy_try_merge(block);
		return 0;
	default:
		block->refcnt--;
		return 0;
	}

	return -1;
}

/* NOTE: this might not give aligned pages? */
struct block *buddy_alloc(unsigned order)
{
	/* Order too large? */
	if (order > BUDDY_MAX_ORDER) return NULL;

	int smallest_free_order = __buddy_find_smallest_free_order(order);

	/* No available blocks? */
	if (smallest_free_order < 0) return NULL;

	/* Split failed? */
	if (buddy_split(smallest_free_order, order)) return NULL;

	return buddy_pop(order);
}

struct block *ppn2block(uintptr_t ppn)
{
	if (ppn < phys2ppn(PHYS_MEM_START) || ppn >= phys2ppn(PHYS_MEM_END)) {
		return NULL;
	} else {
		return __ppn2block(ppn2__ppn(ppn));
	}
}

uintptr_t block2ppn(struct block *block)
{
	return __ppn2ppn(block2__ppn(block));
}

static void buddy_layout_init()
{
	layout.kelf_base = PHYS_MEM_START;
	layout.phys_base = phys_base; /* This one moves, as the kernel grows */
	layout.phys_end = PHYS_MEM_END - PHYS_MEM_KERNEL_STACK_SIZE;
	layout.kstack_size = PHYS_MEM_KERNEL_STACK_SIZE;
}

static void __buddy_init(void)
{
	phys_base = (uintptr_t)&phys_base;

	struct block block = { .refcnt = 1, .order = 0, .next = NULL };

	for (size_t i = 0; i < ARRAY_SIZE(buddy_blocks); i++) {
		buddy_blocks[i] = block;
	}

	for (uintptr_t phys = phys_base;
	     phys < PHYS_MEM_END - PHYS_MEM_KERNEL_STACK_SIZE;
	     phys += PHYS_MEM_PAGE_SIZE) {
		buddy_free(phys2block(phys));
	}
}

static void buddy_reset(void)
{
	memset(buddy_free_lists, 0,
	       sizeof(struct block *) * ARRAY_SIZE(buddy_free_lists));
	memset(buddy_blocks, 0,
	       sizeof(struct block) * ARRAY_SIZE(buddy_blocks));
	__buddy_init();
}

void buddy_migrate(uintptr_t offset)
{
	for (size_t i = 0; i < ARRAY_SIZE(buddy_free_lists); i++) {
		if (!buddy_free_lists[i]) continue;

		buddy_free_lists[i] =
			(struct block *)((char *)buddy_free_lists[i] + offset);
	}

	for (size_t i = 0; i < ARRAY_SIZE(buddy_blocks); i++) {
		struct block *block = &buddy_blocks[i];
		if (block->next) {
			block->next =
				(struct block *)((char *)block->next + offset);
		}
	}
}

void buddy_init(void)
{
	__buddy_init();

	buddy_layout_init();

	printstr("--- buddy layout ---\n");
	printdbg("PHYS_MEM_START       : ", (void *)layout.kelf_base);
	printdbg("phys_base            : ", (void *)layout.phys_base);
	printdbg("phys_end             : ", (void *)layout.phys_end);
	printdbg("PHYS_MEM_END         : ", (void *)PHYS_MEM_END);
	printdbg("kstack_size          : ", (void *)layout.kstack_size);

	printstr("--- buddy meta data ---\n");
	printdbg("buddy_free_lists base: ", (void *)buddy_free_lists);
	printdbg("buddy_free_lists end : ",
		 (char *)buddy_free_lists + sizeof(buddy_free_lists));
	printdbg("buddy_blocks base    : ", (void *)buddy_blocks);
	printdbg("buddy_blocks end     : ",
		 (char *)buddy_blocks + sizeof(buddy_blocks));
}