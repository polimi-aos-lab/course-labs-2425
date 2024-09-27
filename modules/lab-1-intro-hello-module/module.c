#include "linux/printk.h"
#include "linux/stat.h"
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vittorio Zaccaria");
MODULE_DESCRIPTION("Hello AOS students!");

static int num = 5;
module_param(num, int, S_IRUGO);

// insmod /modules/lab-1.... num=10

static int __init hello_init(void) {
  pr_info("Hello world!!");
  pr_info("Address of hello_init = %px", hello_init);
  return 0;
}

static void __exit hello_cleanup(void) { pr_info("Cleanup and exit"); }

module_init(hello_init);
module_exit(hello_cleanup);
