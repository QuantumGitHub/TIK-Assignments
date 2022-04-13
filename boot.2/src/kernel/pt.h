#include "types.h"
#include "util.h"
#include "buddy.h"

#define PT_NUM_LEVELS 5 /* Including SATP */

#define PAGE_SHIFT 12
#define PAGE_SIZE (0x1UL << PAGE_SHIFT)
#define PAGE_OFFSET_MASK 0xFFF

/* Base of kernel virtual address subspace */
#define VIRT_MEM_KERNEL_SHIFT 47
#define VIRT_MEM_KERNEL_BASE (0x1ffffUL << VIRT_MEM_KERNEL_SHIFT)

#define VMA_READ 0x1UL
#define VMA_WRITE 0x2UL
#define VMA_EXEC 0x4UL
#define VMA_POPULATE 0x8UL
#define VMA_IDENTITY 0x10UL

#define PTE_VALID_SHIFT 0
#define PTE_READ_SHIFT 1
#define PTE_WRITE_SHIFT 2
#define PTE_EXEC_SHIFT 3
#define PTE_USER_SHIFT 4
#define PTE_GLOBAL_SHIFT 5 /* Mapping exists in all address spaces (optimis.) */
#define PTE_ACCESSED_SHIFT 6
#define PTE_DIRTY_SHIFT 7

#define __PTE_RSW_SHIFT 8
#define __PTE_RSW_MASK 0x3
#define __PTE_PERM_MASK 0x3FF
#define __PTE_PPN_SHIFT 10
#define __PTE_PPN_MASK 0xFFFFFFFFFFF

#define SATP_PPN_SHIFT 0
#define SATP_PPN_MASK 0xFFFFFFFFFFF

#define l2i(level) (PT_NUM_LEVELS - 1 - (level))
#define i2l(i) l2i(i)

#define pte_get(pte, shift) (((pte) >> (shift)) & 0x1)
#define pte_set(pte, shift) ((pte) | (0x1UL << (shift)))
#define pte_unset(pte, shift) (~pte_set(~(pte), (shift)))

#define flags_get(flags, shift) pte_get(flags, shift)
#define flags_set(flags, shift) pte_set(flags, shift)

#define pte_get_perm(pte) ((pte) & (__PTE_PERM_MASK))
#define pte_clear_perm(pte) ((pte) ^ pte_get_ppn(pte))
#define pte_set_perm(pte, perm) (pte_clear_perm(pte) ^ (perm & __PTE_PERM_MASK))

#define pte_get_ppn(pte) (((pte) >> __PTE_PPN_SHIFT) & __PTE_PPN_MASK)

/* clang-format off */
#define pte_clear_ppn(pte) ((pte) ^ (pte_get_ppn(pte) << __PTE_PPN_SHIFT))
#define pte_set_ppn(pte, new)                                                  \
	(pte_clear_ppn(pte) ^ (((new) & __PTE_PPN_MASK) << __PTE_PPN_SHIFT))
#define satp_clear(satp, mask, shift)                                          \
	((satp) ^ (satp_get(satp, mask, shift) << shift))
#define satp_set(satp, new, mask, shift)                                       \
	(satp_clear(satp, mask, shift) ^ (((new) & (mask)) << (shift)))
/* clang-format on */

#define satp_get(satp, mask, shift) (((satp) >> (shift)) & (mask))
#define satp2pte(satp)                                                         \
	pte_set_ppn(0x1UL, satp_get(satp, SATP_PPN_MASK, SATP_PPN_SHIFT))
#define pte_is_leaf(pte) pte_get(pte, PTE_READ_SHIFT)
#define pte_is_valid(pte) pte_get(pte, PTE_VALID_SHIFT)

#define ppn2phys(ppn) ((ppn) << PAGE_SHIFT)
#define phys2ppn(phys) ((phys) >> PAGE_SHIFT)
#define vpn2virt(vpn) ppn2phys(vpn)
#define virt2vpn(virt) phys2ppn(virt)

struct vma {
	uintptr_t vpn;
	size_t pagen;
	uintptr_t flags;
	uintptr_t __ppn; /* Only applies if identity mapped */
	struct page *page; /* Linked list of pages inside the VMA */
	struct node node;
};

struct page {
	size_t level;
	uintptr_t pte;
	struct block *block;
	struct node node;
};

/**
 *
 * pt_alloc: allocates a page of a certain level (NORMAL, MEGA, etc.)
 *
 * Returns a physical address. Can be accessed (inside the kernel) by using the
 * phys2kvirt() macro
 *
 * NOTE: to be used for large allocations of kernel memory, most often though,
 * you'll want to use pt_salloc (see below)
 *
 */
uintptr_t pt_alloc(size_t level);

/**
 *
 * pt_salloc: allocates a small/slab amount of memory (less than a page)
 *
 * @size: size in bytes, must be less than size of PAGE_NORMAL
 *
 * Returns a pointer in the kernel's address space (ksatp)
 *
 */
void *pt_salloc(size_t size);

/**
 *
 * pt_free: frees a page allocated with pt_alloc()
 *
 * @phys: physical address of the page to be freed
 *
 * Returns 0 on success, -1 on failure
 *
 * NOTE: does not support freeing memory from pt_salloc (that's a TODO)
 *
 */
int pt_free(uintptr_t phys);

/**
 *
 * pt_vma_new: creates a new VMA and maps it
 *
 * @satp: address space to map the VMA into
 * @vpn: virtual page number to be mapped
 * @pagen: size of mapping, in number of pages of size PAGE_NORMAL
 * @flags: permissions and extras such as VMA_POPULATE
 *
 * @out: the VMA will be initialized for you
 *
 * Returns 0 on success
 *
 */
int pt_vma_new(uintptr_t *satp, uintptr_t vpn, size_t pagen, uintptr_t flags,
	       struct vma *out);
int pt_vma_unmap(uintptr_t *satp, struct vma *in);

struct vma *vpn2vma(struct vma *vmas, uintptr_t vpn);
struct page *vma2page(struct vma *vma, const uintptr_t vpn);

void pt_flush_tlb(void);
void *phys2kvirt(uintptr_t phys);

/**
 *
 * pt_get_pte: gets the PTE corresponding to @vpn
 *
 * @satp: address space to search in
 * @vpn: virtual page number to look for
 * @ptes: array of PTEs that will be filled along the way, i.e. page walk
 *
 * @out: will make the pointer you give it point to the PTE it found (which can
 * be used to then change it)
 *
 * Returns the depth "i" at which the PTE was found, can be used to index
 * into @ptes
 *
 */
ssize_t pt_get_pte(uintptr_t *satp, uintptr_t vpn, uintptr_t *ptes,
		   uintptr_t **out);

/* Only used during initialization, you probably don't want to use these */
void pt_jump_to_high(void);
int pt_unmap(void);
int pt_init(void);

#ifdef TEST
int pt_init_kvma(struct vma *out);
#endif
