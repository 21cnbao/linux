#include <linux/printk.h>
void sys_helloworld(void)
{
	printk(KERN_INFO "hello world\n");
}
