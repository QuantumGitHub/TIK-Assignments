// Author: Guillaume Thivolet

#include "stdio.h"
#include <stdint.h>

struct block {
  size_t refcnt;
  uintptr_t order; /* 0 <= order <= BUDDY_MAX_ORDER */
  struct block *next;
};

#define BUDDY_MAX_ORDER 2

struct block *buddy_free_lists[BUDDY_MAX_ORDER + 1];
struct block buddy_blocks[1ULL << BUDDY_MAX_ORDER];

int main() {
  printf("sizeof(struct block) \t\t= %ld\n", sizeof(struct block));
  printf("sizeof(struct block*) \t\t= %ld\n\n", sizeof(struct block *));

  printf("&buddy_blocks \t\t\t= %p\n", buddy_blocks);
  printf("&buddy_blocks[0]\t\t= %p\n", &buddy_blocks[0]);
  printf("&buddy_blocks[1]\t\t= %p\n", &buddy_blocks[1]);
  printf("&buddy_blocks[2]\t\t= %p\n", &buddy_blocks[2]);
  printf("&buddy_blocks[3]\t\t= %p\n\n", &buddy_blocks[3]);

  buddy_blocks[0].next = 0;
  buddy_blocks[0].refcnt = 0;
  buddy_blocks[0].order = 0;

  // undefined behavior
  printf("UB> buddy_blocks[0]\t\t= %zu\n", *buddy_blocks);
  printf("buddy_blocks[0].refcnt\t\t= %zu\n", buddy_blocks[0].refcnt);

  // All 3 following lines: undefined behavior  (UB)
  printf("UB> &buddy_blocks[10]\t\t= %p\n", &buddy_blocks[10]);
  buddy_blocks[10].order = 0;
  printf("UB> buddy_blocks[10]\t\t= %lu\n\n", buddy_blocks[10].order);

  printf("&buddy_free_lists\t\t= %p\n", buddy_free_lists);
  printf("&buddy_free_lists[0]\t\t= %p\n", &(buddy_free_lists[0]));
  printf("&buddy_free_lists[1]\t\t= %p\n", &(buddy_free_lists[1]));
  printf("&buddy_free_lists[2]\t\t= %p\n\n", &(buddy_free_lists[2]));

  printf("buddy_free_lists[0])\t\t= %p\n", buddy_free_lists[0]);
  printf("buddy_free_lists[1])\t\t= %p\n", buddy_free_lists[1]);
  printf("buddy_free_lists[2])\t\t= %p\n\n", buddy_free_lists[2]);

  buddy_free_lists[0] = buddy_blocks;
  buddy_free_lists[1] = buddy_blocks + 1;

  printf("buddy_free_lists[0]\t\t= %p\n", buddy_free_lists[0]);
  printf("buddy_free_lists[1]\t\t= %p\n", buddy_free_lists[1]);

  // question for students: why isn't the result 0x30?
  // answer: pointer addition is not done on byte size but element size  (ie. p+a would
  // be translated in bytes to p+a*sizeof(p))
  printf("&buddy_blocks[2] - buddy_blocks\t= %ld\n",
         &(buddy_blocks[2]) - buddy_blocks);

  return 0;
}
