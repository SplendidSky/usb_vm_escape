#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define BULK_EP_OUT 0x01
#define BULK_EP_IN 0x82
#define MAX_PKT_SIZE 512

static struct usb_device *device;
static struct usb_class_driver class;
static unsigned char bulk_buf[MAX_PKT_SIZE];

static int tablet_open(struct inode *i, struct file *f)
{
    return 0;
}
static int tablet_close(struct inode *i, struct file *f)
{
    return 0;
}

static ssize_t tablet_read(struct file *f, char __user *buf, size_t cnt, loff_t *off)
{
    int retval;
    int read_cnt;

    /* Read the data from the bulk endpoint */
    retval = usb_bulk_msg(device, usb_rcvbulkpipe(device, BULK_EP_IN),
            bulk_buf, MAX_PKT_SIZE, &read_cnt, 5000);
    if (retval)
    {
        printk(KERN_ERR "Bulk message returned %d\n", retval);
        return retval;
    }
    if (copy_to_user(buf, bulk_buf, MIN(cnt, read_cnt)))
    {
        return -EFAULT;
    }

    return MIN(cnt, read_cnt);
}

static ssize_t tablet_write(struct file *f, const char __user *buf, size_t cnt,
                                    loff_t *off)
{
    int retval;
    int wrote_cnt = MIN(cnt, MAX_PKT_SIZE);

    if (copy_from_user(bulk_buf, buf, MIN(cnt, MAX_PKT_SIZE)))
    {
        return -EFAULT;
    }

    /* Write the data into the bulk endpoint */
    retval = usb_bulk_msg(device, usb_sndbulkpipe(device, BULK_EP_OUT),
            bulk_buf, MIN(cnt, MAX_PKT_SIZE), &wrote_cnt, 5000);
    if (retval)
    {
        printk(KERN_ERR "Bulk message returned %d\n", retval);
        return retval;
    }

    return wrote_cnt;
}

static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .open = tablet_open,
    .release = tablet_close,
    .read = tablet_read,
    .write = tablet_write,
};

static int tablet_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    int retval;

    device = interface_to_usbdev(interface);

    class.name = "usb/tablet%d";
    class.fops = &fops;
    if ((retval = usb_register_dev(interface, &class)) < 0)
    {
        /* Something prevented us from registering this driver */
        printk(KERN_ERR "Not able to get a minor for this device.");
    }
    else
    {
        printk(KERN_INFO "Minor obtained: %d\n", interface->minor);
    }

    return retval;
}

static void tablet_disconnect(struct usb_interface *interface)
{
    printk(KERN_INFO "Tablet drive removed\n");
}

static struct usb_device_id tablet_table[] =
{
    { USB_DEVICE(0x0627, 0x0001) },
    {} /* Terminating entry */
};
MODULE_DEVICE_TABLE (usb, tablet_table);


static struct usb_driver tablet_driver =
{
    .name = "tablet_driver",
    .id_table = tablet_table,
    .probe = tablet_probe,
    .disconnect = tablet_disconnect,
};

static int __init tablet_init(void)
{
    /* Register this driver with the USB subsystem */
    if ((result = usb_register(&tablet_driver)))
    {
        printk(KERN_ERR "usb_register failed. Error number %d", result);
    }
    return result;
}

static void __exit tablet_exit(void)
{
    usb_deregister(&tablet_driver);
}

module_init(tablet_init);
module_exit(tablet_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tiange <splendidsky.cwc@alibaba-inc.com>");
MODULE_DESCRIPTION("USB Tablet Registration Driver");