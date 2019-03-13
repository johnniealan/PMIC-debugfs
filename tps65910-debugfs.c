/*
 * TPS register - debugfs
 *
 * Author: Johnnie Alan <johnniealan@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#include <linux/mfd/core.h>
#include <linux/mfd/tps65910.h>
#include <linux/of_device.h>
static struct dentry *regmap_debugfs_root;
struct mutex lock;

static ssize_t tps65910_write_register(struct file *file,
                                     const char __user *user_buf,
                                     size_t count, loff_t *ppos)
{
        unsigned int val =0;
#ifdef DEBUG_TPS
        unsigned int i =0;
#endif
        char *buf = NULL;
        struct tps65910 *tps = file->private_data;
        struct tps_rdw *tps_data = NULL;

        if (*ppos < 0 || !count)
                return -EINVAL;

        buf = kmalloc(sizeof(struct tps_rdw),GFP_KERNEL);
        if (!buf)
            return -ENOMEM;

        if ( copy_from_user(buf,user_buf,sizeof(struct tps_rdw)) ) {
                 kfree(buf);
                 return -EFAULT;
         }

        mutex_lock(&lock);
        tps_data = (struct tps_rdw *) buf;
#ifdef DEBUG_TPS
        printk("\nReg=%2x,Value =%2x\n",tps_data->reg,val);
#endif
        memcpy(&val,&tps_data->value,1);
        tps65910_reg_write(tps,tps_data->reg, val);

#ifdef DEBUG_TPS
        for(i=0;i<count;i++)
        {
            printk("%x\t",buf[i]);
        }
        printk("\n");
#endif
        mutex_unlock(&lock);
        kfree(buf);
  return count;
}
static ssize_t tps65910_read_register(struct file *file,
                                   char __user *user_buf,
                                   size_t count, loff_t *ppos)
{
        unsigned int val =0;
        char *buf = NULL;
#ifdef DEBUG_TPS
        unsigned int i =0;
#endif
        struct tps65910 *tps = file->private_data;
        struct tps_rdw *tps_data = NULL;

        if (*ppos < 0 || !count)
                return -EINVAL;

        buf = kmalloc(sizeof(struct tps_rdw),GFP_KERNEL);
        if (!buf)
            return -ENOMEM;

        if ( copy_from_user(buf,user_buf,sizeof(struct tps_rdw)) ) {
                 kfree(buf);
                 return -EFAULT;
         }

        mutex_lock(&lock);
        tps_data = (struct tps_rdw *) buf;
#ifdef DEBUG_TPS
        printk("Reg=%2x\n",tps_data->reg);
#endif
        tps65910_reg_read(tps,tps_data->reg, &val);
        memcpy(&tps_data->value,&val,1); 
#ifdef DEBUG_TPS
        printk("Driver Read::: Val=%2x, val=%2x\n",tps_data->value,val);
        for(i=0;i<count;i++)
        {
            printk("%x\t",buf[i]);
        }
        printk("\n");
#endif
        if (copy_to_user(user_buf, buf, sizeof(struct tps_rdw))) {
                 kfree(buf);
                 return -EFAULT;
         }
        *ppos += count;
        mutex_unlock(&lock);
        kfree(buf);
  return count;
}

void pmic_debugfs_exit(void)
{
    if (regmap_debugfs_root) {
            debugfs_remove_recursive(regmap_debugfs_root);
    }
}
static const struct file_operations tps65910_fops = {
        .open = simple_open,
        .read = tps65910_read_register,
        .write = tps65910_write_register,
        .llseek = default_llseek,
};
void pmic_debugfs_init(struct tps65910 *tps)
{
   regmap_debugfs_root = debugfs_create_dir("pmic", NULL);
   if (!regmap_debugfs_root) {
                pr_warn("regmap: Failed to create debugfs root\n");
                return;
   }
   mutex_init(&lock);
   debugfs_create_file("tps", 0400, regmap_debugfs_root,
                             tps, &tps65910_fops);
}
