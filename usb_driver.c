#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define BULK_EP_OUT 0x01
#define BULK_EP_IN 0x81
#define MAX_PKT_SIZE 512

static struct usb_device *device;
static struct usb_class_driver class;
static unsigned char my_buf[MAX_PKT_SIZE];
static struct urb *my_urb;

static void tablet_irq(struct urb *urb)
{
    printk(KERN_ERR "recv msg: %s\n", my_buf);
    usb_submit_urb(my_urb, GFP_KERNEL);
}


static int tablet_open(struct inode *i, struct file *f)
{
    return 0;
}
static int tablet_close(struct inode *i, struct file *f)
{
    return 0;
}

static int tablet_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    int retval;
    struct usb_endpoint_descriptor *endpoint;
    struct usb_host_interface *interface;
    int pipe;

    device = interface_to_usbdev(interface);
    interface=intf->cur_altsetting;
    endpoint = &interface->endpoint[0].desc;
   
    printk(KERN_INFO "VID=%x,PID=%x\n",device->descriptor.idVendor,dev->descriptor.idProduct);

    pipe = usb_rcvintpipe(dev,endpoint->bEndpointAddress);

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

    my_urb = usb_alloc_urb(0, GFP_KERNEL);
    usb_fill_int_urb(my_urb, device, pipe, my_buf, MAX_PKT_SIZE, tablet_irq, 0, endpoint->bInterval);
    usb_submit_urb(my_urb, GFP_KERNEL);

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
    int result;
    
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