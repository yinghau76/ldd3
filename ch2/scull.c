#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>

MODULE_LICENSE("Dual BSD/GPL");

#define TAG "scull: "

struct scull_dev {
    struct scull_qset* data;
    int quantum;
    int qset;
    unsigned long size;
    unsigned int access_key;
    struct cdev cdev;
};

static int scull_trim(struct scull_dev* dev) {
    printk(KERN_ALERT TAG "trim");
    return 0;
}

static int scull_open(struct inode* inode, struct file* filp) {
    // setup private data for later use
    struct scull_dev* dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev;

    // handle write-only mode
    if ((filp->f_flags & O_ACCMODE) == O_WRONLY) {
        scull_trim(dev);
    }
    return 0;
}

struct file_operations scull_fops = {
    .owner = THIS_MODULE,
    .open = scull_open,
};

static int scull_major = 0;
static int scull_minor = 0;
static int scull_nr_devs = 1;

static int alloc_dev_number(void) {
    dev_t dev;
    int result;

    if (scull_major) {
        dev = MKDEV(scull_major, scull_minor);
        result = register_chrdev_region(dev, scull_nr_devs, "scull");
    } else {
        result = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs, "scull");
        scull_major = MAJOR(dev);
    }

    if (result < 0) {
        printk(KERN_WARNING TAG "can't get major %d\n", scull_major);
        return result;
    }

    return 0;
}

static void scull_setup_cdev(struct scull_dev* dev, int index) {
    int err, devno = MKDEV(scull_major, scull_minor + index);

    cdev_init(&dev->cdev, &scull_fops);
    dev->cdev.owner = THIS_MODULE;
    err = cdev_add(&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_NOTICE "Error %d adding scull%d", err, index);
    }
}

static struct scull_dev g_dev;

static __init int scull_init(void) {
    printk(KERN_ALERT TAG "init\n");

    alloc_dev_number();
    scull_setup_cdev(&g_dev, 0);

    return 0;
}

static __exit void scull_exit(void) {
    printk(KERN_ALERT TAG "exit\n");
}

module_init(scull_init);
module_exit(scull_exit);

