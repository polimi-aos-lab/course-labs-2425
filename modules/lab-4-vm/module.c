#include <asm/io.h>   // virt_to_phys
#include <linux/mm.h> // vmalloc_to_page etc..
#include <linux/module.h>
#include <linux/random.h>
#include <linux/slab.h>     // kmalloc, zones etc..
#include <linux/slub_def.h> // kmalloc
#include <linux/vmalloc.h>  //vmalloc

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vittorio Zaccaria");
MODULE_DESCRIPTION("Simple memory alloc, with kmalloc and vmalloc");

static void *buffer;

#define PN(x) ((void *)((unsigned long long)(x) >> PAGE_SHIFT))

static int print_zones(void) {
  int node_id = numa_node_id();
  struct pglist_data *pgdat = NODE_DATA(node_id);
  int zone_id;

  pr_info("Memory Zones for NUMA Node %d:\n", node_id);

  for (zone_id = 0; zone_id < MAX_NR_ZONES; zone_id++) {
    struct zone *zone = &pgdat->node_zones[zone_id];

    if (zone->present_pages) {
      unsigned long start_pfn = zone->zone_start_pfn;
      unsigned long end_pfn = zone_end_pfn(zone);

      pr_info("Zone %d - Start PPN: 0x%lx, End PPN: 0x%lx\n", zone_id,
              start_pfn, end_pfn);
    }
  }
  return 0;
}

static int alloc_kmalloc(int n) {
  // Calculate the size required for a buffer spanning two pages
  size_t buffer_size = n * PAGE_SIZE;

  // Allocate the buffer with kmalloc
  buffer = kmalloc(buffer_size, GFP_KERNEL);
  if (!buffer) {
    pr_info(KERN_ERR "Failed to allocate the buffer\n");
    return -ENOMEM;
  }

  // Get the physical addresses of the first and second pages

  pr_info("kmalloc - VPN: %px -> PPN: %px\n\n", PN(buffer),
          PN(virt_to_phys(buffer)));
  kfree(buffer);
  return 0;
}

static int alloc_vmalloc(int n) {
  // Calculate the size required for a buffer spanning two pages
  size_t buffer_size = n * PAGE_SIZE;
  int i;

  // Allocate the buffer with kmalloc
  buffer = vmalloc(buffer_size);
  for (i = 0; i < n; i++) {
    struct page *page = vmalloc_to_page(buffer + i * PAGE_SIZE);
    unsigned long ppn = page_to_pfn(page);
    pr_info("vmalloc - VPN: %px -> PPN: %px\n", PN(buffer + i * PAGE_SIZE),
            (void *)ppn);
  }
  vfree(buffer);
  return 0;
}

#define NR_TRIES 30
#define C_SIZE 700

static void *random_us_ptr(void) {
  u64 rand_val;
  get_random_bytes(&rand_val, sizeof(rand_val));
  rand_val = rand_val & 0x00ffffff;
  return (void *)(uintptr_t)rand_val;
}

static void print_proc_info(void) {
  struct mm_struct *mm;
  struct vm_area_struct *vma, *vmaf;
  int i;
  char to[C_SIZE];
  pr_info("Current process %s\n", current->comm);
  mm = current->mm;
  vmaf = mm->mmap;
  for (vma = vmaf; vma; vma = vma->vm_next) {
    vmaf = vma;
    pr_info("VMA: 0x%lx - 0x%lx %c\n", vma->vm_start, vma->vm_end,
            vma->vm_flags & VM_READ ? 'R' : 'N');
  }
  // Try to access randomnly user address space addresses.
  // copy_from_user just returns 0 (of C_SIZE) bytes copied instead
  // of crashing the kernel on invalid user space addresses.
  for (i = 0; i < NR_TRIES; i++) {
    int res;
    const void *f = random_us_ptr();
    res = copy_from_user(to, f, C_SIZE);
    pr_info("We survived...accessing %px, read %d bytes\n", f, C_SIZE - res);
  }
}

struct my_struct {
  u64 field1;
  u64 field2;
  u8 field3;
  spinlock_t lock;
} my_struct;

static int howmany = 0;

static void my_struct_constructor(void *addr) {
  struct my_struct *p = (struct my_struct *)addr;
  memset(p, 0, sizeof(struct my_struct));
  spin_lock_init(&p->lock);
  howmany++;
  pr_info("my_struct_constructor: %d \n", howmany);
}

#define NUM_OBJ 19

struct kmem_cache *cc;
typedef struct my_struct *my_struct_p;
static my_struct_p store[NUM_OBJ];

static void build_and_fill_kmem_cache(void) {
  int i;
  cc = kmem_cache_create(
      "my_struct", sizeof(struct my_struct), 0, SLAB_HWCACHE_ALIGN,
      my_struct_constructor); // note that you dont have a destructor!
  for (i = 0; i < NUM_OBJ; i++) {
    store[i] = kmem_cache_alloc(cc, GFP_KERNEL);
    pr_info("kmem_cache_alloc: %d \n", i);
  }
}

static int __init memalloc_init(void) {
  print_zones();
  pr_info("Kernel logical base VPN: %px", PN(PAGE_OFFSET));
  pr_info("Kernel virtual range (VPN - VPN): %px - %px", PN(VMALLOC_START),
          PN(VMALLOC_END));
  alloc_kmalloc(4);
  alloc_vmalloc(4);
  print_proc_info();
  build_and_fill_kmem_cache();
  return 0;
}

static void __exit memalloc_cleanup(void) {
  int i;
  for (i = 0; i < NUM_OBJ; i++) {
    kmem_cache_free(cc, store[i]);
    pr_info("kmem_cache_free: %d \n", i);
  }
}

module_init(memalloc_init);
module_exit(memalloc_cleanup);
