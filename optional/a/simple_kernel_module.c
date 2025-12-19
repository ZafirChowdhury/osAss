#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hash.h>
#include <linux/gcd.h>


static int simple_init(void)
{
    printk(KERN_INFO "Loading Kernel Module\n");
    printk(KERN_INFO "Golden Ratio Prime: %llu\n", GOLDEN_RATIO_PRIME);
    
    return 0;
}


static void simple_exit(void)
{
    unsigned long result = gcd(3700, 24);
    
    printk(KERN_INFO "GCD of 3700 and 24: %lu\n", result);
    printk(KERN_INFO "Removing Kernel Module\n");
}

module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("Zafir");
