/*
 * linux/drivers/char/skbd.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/input.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#define MAX_KEY_CNT 256
#define DEVICE_NAME "skbd"

static struct input_dev *input;

static unsigned char skbd_keycode[MAX_KEY_CNT] = {
	[','] = KEY_LEFT,
	['.'] = KEY_RIGHT,
	['r'] = KEY_ENTER,
	['a'] = KEY_F1,
	['b'] = KEY_F2,
	['c'] = KEY_F3,
	['d'] = KEY_F4,
	['e'] = KEY_F5,
	['f'] = KEY_F6,
	['g'] = KEY_F7,
	['h'] = KEY_F8,
	['i'] = KEY_F9,
	['j'] = KEY_F10,
	['k'] = KEY_F11,
	['m'] = KEY_F12,
	['n'] = KEY_F13,
	['p'] = KEY_F14,
	['q'] = KEY_F15,
};

static ssize_t skbd_write(struct file *filp, const char __user *buf, size_t len, loff_t *offset)
{
	unsigned char index;

	if (len != 2) {
		return len;
	}

	index = buf[0] % MAX_KEY_CNT;

	if (skbd_keycode[index]) {
		input_report_key(input, skbd_keycode[index], buf[1]);
		input_sync(input);
	} else {
		/* printk(KERN_WARNING "skbd.c: Unknown key (scancode %#x).\n", data); */
	}

	return len;
}

static struct file_operations skbd_dev_fops = {
	.owner   = THIS_MODULE,
	.write   = skbd_write,
};

static struct miscdevice skbd_dev= {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEVICE_NAME,
	.fops	= &skbd_dev_fops,
};

static int __init skbd_init(void)
{
	int i;

	input = input_allocate_device();
	if (!input)
		return -ENOMEM;

	set_bit(EV_KEY, input->evbit);

	for (i = 0; i < MAX_KEY_CNT; i++)
		set_bit(skbd_keycode[i], input->keybit);

	input->name = "Serial Keyboard";
	input->phys = "skbd/input0";

	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0001;
	input->id.product = 0x0001;
	input->id.version = 0x0100;
	input->keycode = skbd_keycode;

	if (input_register_device(input) != 0)
	{
		printk("skbd input device register fail!\n");

		input_free_device(input);
		return -ENODEV;
	}

	return misc_register(&skbd_dev);
}

static void __exit skbd_exit(void)
{
	input_unregister_device(input);
	misc_deregister(&skbd_dev);
}

module_init(skbd_init);
module_exit(skbd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BLM-TECH Inc.");
