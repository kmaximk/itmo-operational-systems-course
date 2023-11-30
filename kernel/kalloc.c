// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "constants.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "types.h"

#define POS(a) \
  (PGROUNDDOWN((uint64)a) / PGSIZE) - (PGROUNDUP((uint64)end) / PGSIZE)

void freerange(void *pa_start, void *pa_end);

extern char end[];  // first address after kernel.
                    // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct {
  struct spinlock lock;
  uint32 counter[PHYSTOP / PGSIZE];
} ref_cnt;

void kinit() {
  initlock(&kmem.lock, "kmem");
  initlock(&ref_cnt.lock, "ref_cnt");
  freerange((void *)end, (void *)PHYSTOP);
}

void freerange(void *pa_start, void *pa_end) {
  char *p;
  p = (char *)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE) {
    ref_cnt.counter[POS(p)] = 1;
    kfree(p);
  }
}

void counter_change(void *pa, int val) {
  acquire(&ref_cnt.lock);
  if (ref_cnt.counter[POS(pa)] == 1 && val == -1) {
    release(&ref_cnt.lock);
    kfree(pa);
    return;
  }
  ref_cnt.counter[POS(pa)] += val;
  release(&ref_cnt.lock);
}

uint32 counter_get(void *pa) { return ref_cnt.counter[POS(pa)]; }

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void *pa) {
  struct run *r;

  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&ref_cnt.lock);
  ref_cnt.counter[POS(pa)]--;
  if (ref_cnt.counter[POS(pa)] > 0) {
    release(&ref_cnt.lock);
    return;
  }
  release(&ref_cnt.lock);

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run *)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *kalloc(void) {
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if (r) kmem.freelist = r->next;
  release(&kmem.lock);

  if (r) {
    acquire(&ref_cnt.lock);
    ref_cnt.counter[POS(r)] = 1;
    release(&ref_cnt.lock);
  }

  if (r) memset((char *)r, 5, PGSIZE);  // fill with junk
  return (void *)r;
}
