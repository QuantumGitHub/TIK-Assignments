#include "buddy.h"

#define PHYS_MEM_ORDER 25 /* 32 MB */
#define PHYS_MEM_SIZE (1UL << PHYS_MEM_ORDER)

#define PHYS_MEM_PAGE_ORDER 12
#define PHYS_MEM_PAGE_SIZE (1UL << PHYS_MEM_PAGE_ORDER)

#define PHYS_MEM_START 0x80000000
#define PHYS_MEM_END (PHYS_MEM_START + PHYS_MEM_SIZE)
#define PHYS_MEM_KERNEL_STACK_SIZE (1 * PHYS_MEM_PAGE_SIZE)

/* Buddy-specific macros */
#define BUDDY_MAX_ORDER (PHYS_MEM_ORDER - PHYS_MEM_PAGE_ORDER)

/* | 0x80000000                        |                        |             |
 * | .text segments... .text of buddy  | buddy's .data and .bss | -->         |
 * |                                   |                     <-- kernel stack |
 */

/* __ppn is buddy's internal ppn, with __ppn == 0 for PHYS_MEM_START */
#define phys2ppn(phys) ((phys) >> PHYS_MEM_PAGE_ORDER)

#define phys2__ppn(phys) (((phys)-PHYS_MEM_START) >> PHYS_MEM_PAGE_ORDER)

#define __ppn2ppn(__ppn) ((__ppn) + phys2ppn(PHYS_MEM_START))
#define ppn2__ppn(ppn) ((ppn)-phys2ppn(PHYS_MEM_START))

#define __ppn2block(__ppn) ((__ppn) + buddy_blocks)
#define block2__ppn(block) ((block)-buddy_blocks)

#define phys2block(phys) __ppn2block(phys2__ppn(phys))

/*
 * TODO: start by implementing block2buddy(block, order), where order is the
 * order of block. Use __ppn2block and block2__ppn
 */
/* #define block2buddy(block, order) */
#define block2buddy(block, order) (__ppn2block(block2__ppn(block)^(1UL << order)))

#define is_list_empty(order) (!(uintptr_t)buddy_free_lists[order])
#define is_block_free(block) (!(block->refcnt))

static struct block *buddy_free_lists[BUDDY_MAX_ORDER + 1];
static struct block buddy_blocks[1UL << BUDDY_MAX_ORDER];

struct buddy_layout layout;

extern uintptr_t phys_base;

static void buddy_print_status()
{
	for (size_t i = 0; i < ARRAY_SIZE(buddy_free_lists); i++) {
		size_t free_blocks = 0;

		printptr((void *)i, '\n');

		if (!is_list_empty(i)) {
			struct block *block = buddy_free_lists[i];
			do {
				free_blocks++;
			} while (block->next);
		}

		printptr((void *)free_blocks, '\n');
		printstr("----------------\n");
	}
}

static struct block *buddy_pop(unsigned order)
{
	if (is_list_empty(order)) return NULL;

	struct block *block = buddy_free_lists[order];

	/* TODO: What should happen here? */
	// take the first element of the order's list and save it into block,
	// then make the next element the new first one
	
	buddy_free_lists[order] = buddy_free_lists[order]->next;

	return block;
}

static struct block *buddy_remove(struct block *block, unsigned order)
{
	if (is_list_empty(order)) return NULL;

	if (buddy_free_lists[order] != block) { //if block not in list create:
		struct block *head = buddy_free_lists[order]; // a head pointer pointing on the element of buddy list of order
		struct block *tail = head;				// the tail of the block, currently pointing on the head
		struct block *prev = NULL;				// prev???
												// __________________________
												// |						|
												// | head					| tail
												// |						|
												// __________________________
		while (tail->next) { //as long as tail points to a next block
			if (tail->next == block) prev = tail; //if the tail points to our block from the argument, set prev = tail
			tail = tail->next;
		} // will terminate if next is NULL, means we have got to the end of the current block orders block

		if (!prev) return NULL; /* Could not find block */

		buddy_free_lists[order] = block; // set the freed block as the first element of the buddy list of order
		prev->next = NULL; // why???
		tail->next = head; // set the tail->next to the heads value???
	}

	return buddy_pop(order); // buddy_pop entfernt das erste element aus dem buddy_free_lists[order]
}

static void buddy_push(struct block *block, unsigned order)
{
	/*
	 * TODO: what should happen here? Hint: at least 2 lines. Hint: insert
	 * the block in front of the linked list it belongs to
	 */
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

	/* TODO: implement the actual splitting here */

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
	 * TODO: implement the actual merging here. Think about which of these
	 * blocks will become "the larger one". Make use of buddy_remove and
	 * buddy_pop. Return the merged block
	 */

	struct block *first; //important to use pointers
	struct block *second;

	//wenn wir am rand hinten sind
	if(block->next == NULL || buddy->next == NULL){ //entscheiden welcher block im speicher früher kommt
		if(block->next == NULL) {
			first = buddy;
			second = block;
		}
		else if(buddy->next == NULL){
			first = block; 
			second = buddy;
		}
	}
	else{
		//wenn wir irgendwo im speicher sind
		if(block->next < buddy->next){
			first = block;
			second = buddy;
		}
		else if(block->next > buddy->next){
			first = buddy;
			second = block;
		}
	}

	first->next = second->next;
	buddy_remove(second, second->order);
	first->order++;

	return first;
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
		/*
		 * TODO: what should happen here? Hint: use buddy_push. Hint:
		 * also use __buddy_try_merge after you've implemented
		 * __buddy_merge
		 */
		return 0;
	default:
		/* TODO: and here? */
		return 0;
	}

	return -1;
}

/* NOTE: this might not give aligned pages? */
struct block *buddy_alloc(unsigned order)
{
	/* Order too large? */
	if (order > BUDDY_MAX_ORDER) return NULL;

	/* TODO: call __buddy_find_smallest_free_order here */

	/*
	 * TODO: think about what __buddy_find_smallest_free_order might
	 * return and what should happen in each case (return early upon a
	 * failure)
	 */

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
		/* TODO: fix buddy_free to make initialization work */
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