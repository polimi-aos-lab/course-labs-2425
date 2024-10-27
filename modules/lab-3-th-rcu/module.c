#include <linux/delay.h> // msleep
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/slab.h> // kmalloc

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vittorio Zaccaria");

static int variant = 0;
// insmod /modules/.... type = 0 # runs broken version
// insmod /modules/.... type = 1 # runs fixed one (the one with rcu)
module_param(variant, int, S_IRUGO);

// Define a structure for the list elements
struct list_element {
  int data;
  struct list_head list;
};

// Define the list and the lock for synchronization
static LIST_HEAD(my_list);
static DEFINE_SPINLOCK(list_lock);

// This kthread reads and prints the shared list.
// Notably, spinlock operations that would normally protect against concurrent
// modification are executed and this should create an inconsistency and oops
// the kernel
static int read_list_thread_norcu(void *data) {
  while (!kthread_should_stop()) {
    struct list_element *entry;
    pr_info("[ ");
    list_for_each_entry(entry, &my_list, list) { pr_info("%d ", entry->data); }
    pr_info("] \n");
    msleep(100); // Sleep for 1 second
  }
  return 0;
}

// This thread manipulates the shared list by
// removing the first element (if any), incrementing its value and adding it
// back. Here we use a spinlock for concurrent writes
static int manipulate_list_thread_norcu(void *data) {
  while (!kthread_should_stop()) {
    struct list_element *entry, *temp;
    entry = kmalloc(sizeof(struct list_element), GFP_KERNEL);
    spin_lock(&list_lock);
    if (!list_empty(&my_list)) {
      temp = list_first_entry(&my_list, struct list_element, list);
      list_del(&temp->list);
      entry->data = temp->data + 1;
      kfree(temp);
    }
    list_add(&entry->list, &my_list);
    spin_unlock(&list_lock);
    msleep(200); // Sleep for 2 seconds
  }
  return 0;
}

// read_list_thread_rcu() works similarly to read_list_thread(), but uses RCU
// read-side critical section (via rcu_read_lock() and rcu_read_unlock()) for
// safe iteration through the shared list.
static int read_list_thread_rcu(void *data) {
  while (!kthread_should_stop()) {
    struct list_element *entry;
    rcu_read_lock();
    pr_info("[ ");
    list_for_each_entry(entry, &my_list, list) { pr_info("%d ", entry->data); }
    pr_info("] \n");
    rcu_read_unlock();
    msleep(100); // Sleep for 1 second
  }
  return 0;
}

struct rcu_head pending_deletes;

// manipulate_list_thread_rcu() works similarly to the non-rcu one,
// but it uses RCU update-side primitives for safe manipulation of the shared
// list. This includes deleting an element with list_del_rcu(), waiting until
// all readers have finished with synchronize_rcu() before kfreeing it, and
// finally adding an element back with list_add_rcu().
// Note, we use spinlock only for concurrent writes
static int manipulate_list_thread_rcu(void *data) {
  while (!kthread_should_stop()) {
    struct list_element *entry, *temp;
    spin_lock(&list_lock);
    entry = kmalloc(sizeof(struct list_element), GFP_KERNEL);
    if (!list_empty(&my_list)) {
      temp = list_first_entry(&my_list, struct list_element, list);
      list_del_rcu(&temp->list);
      entry->data = temp->data + 1;
      synchronize_rcu();
      kfree(temp);
    }
    list_add_rcu(&entry->list, &my_list);
    spin_unlock(&list_lock);
    msleep(200); // Sleep for 2 seconds
  }
  return 0;
}

static struct task_struct *read_thread;
static struct task_struct *manipulate_thread;

static void initialize_list(void) {
  int i;

  // Initialize the list with 10 elements
  for (i = 0; i < 10; i++) {
    struct list_element *entry =
        kmalloc(sizeof(struct list_element), GFP_KERNEL);
    entry->data = i;
    INIT_LIST_HEAD(&entry->list);
    spin_lock(&list_lock);
    list_add_tail(&entry->list, &my_list);
    spin_unlock(&list_lock);
  }
}

void broken_list_manip(void) {
  read_thread = kthread_run(read_list_thread_norcu, NULL, "read_list_thread");
  manipulate_thread =
      kthread_run(manipulate_list_thread_norcu, NULL, "manipulate_list_thread");
}

void fixed_list_manip(void) {
  read_thread = kthread_run(read_list_thread_rcu, NULL, "read_list_thread");
  manipulate_thread =
      kthread_run(manipulate_list_thread_rcu, NULL, "manipulate_list_thread");
}

static int __init my_module_init(void) {
  initialize_list();
  if (variant == 0) {
    broken_list_manip(); // this should at least oops the kernel
  } else {
    fixed_list_manip();
  }

  if (read_thread && manipulate_thread) {
    pr_info("Kernel threads created and started\n");
  } else {
    pr_info(KERN_ERR "Failed to create kernel threads\n");
    return -ENOMEM;
  }

  return 0;
}

static void __exit my_module_exit(void) {
  struct list_element *entry, *temp;
  if (read_thread && manipulate_thread) {
    kthread_stop(read_thread);
    kthread_stop(manipulate_thread);
  }

  // Clean up the list. Note that the stop signal could be
  // delivered later on, so we should synchronize 'list_del'
  spin_lock(&list_lock);
  list_for_each_entry_safe(entry, temp, &my_list, list) {
    list_del(&entry->list);
    kfree(entry);
  }
  spin_unlock(&list_lock);

  pr_info("Kernel threads stopped and module unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
