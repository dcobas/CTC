#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <vmebus.h>

#define	CTC_NCHANS	8
#define MAX_DEVICES	32
#define DRIVER_NAME	"ctc"
#define PFX		DRIVER_NAME ": "

struct ctc_channel {
	uint32_t	conf;
	uint32_t	delay1;
	uint32_t	delay2;
	uint32_t	out_count;
	uint32_t	value1;
	uint32_t	value2;
};

struct ctc_registers {
	uint32_t		hw_stat;
	uint32_t		count_enb;
	struct ctc_channel	channel[CTC_NCHANS];
};

static struct ctc_device {
	int			lun;
	struct vme_mapping	*mapping;
	struct ctc_registers	*regs;
} devices[MAX_DEVICES];

static dev_t ctc_major;
struct file_operations ctc_fops;

static unsigned int lun_num;
static long lun[MAX_DEVICES];
module_param_array(lun, long, &lun_num, S_IRUGO);
MODULE_PARM_DESC(lun, "Logical unit numbers");

static unsigned int base_address_num;
static long base_address[MAX_DEVICES];
module_param_array(base_address, long, &base_address_num, S_IRUGO);
MODULE_PARM_DESC(base_address, "First map base addresses");

static int check_module_params(void)
{
	if (lun_num <= 0 || lun_num > MAX_DEVICES) {
		printk(PFX "bad number of logical units defined.\n");
		return -EACCES;
	}
	if (base_address_num != lun_num) {
		printk(PFX "missing first base address.\n");
		return -EACCES;
	}
	return 0;
}

int install_device(struct ctc_device *dev, unsigned i)
{
	struct vme_mapping desc, *descp = &desc;
	memset(dev, 0, sizeof(*dev));

	dev->lun = lun[i];

	/* configure mmapped I/O */
	descp->data_width = VME_D32;
	descp->am = VME_A24_USER_DATA_SCT;
	descp->vme_addru = 0;
	descp->vme_addrl = base_address[i];
	descp->sizeu = 0;
	descp->sizel = 200;
	descp->read_prefetch_enabled = 0;
	if (vme_find_mapping(descp, 1) != 0) {
		printk(KERN_ERR PFX "could not map lun %d\n", dev->lun);
		goto out_map;
	}
	dev->regs = (struct ctc_registers *)descp->kernel_va;
	return 0;

out_map:
	return -ENODEV;
}

static void unregister_module(struct ctc_device *dev)
{
	vme_release_mapping(dev->mapping, 1);
}


static int ctc_open(struct inode *ip, struct file *filp)
{
	long minor;
	int i;

	minor = MINOR(ip->i_rdev);
	for (i = 0; i < lun_num; i++) {
		if (devices[i].lun == minor) {
			filp->private_data = &devices[i];
			return 0;
		}
	}
	return -ENODEV;
}

static int ctc_ioctl32(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct ctc_device *dev = filp->private_data;

}

static int ctc_close(struct inode *ip, struct file *filp)
{
	filp->private_data = NULL;
}

struct file_operations ctc_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = ctc_ioctl32,
	.open = ctc_open,
	.release = ctc_close,
};

static int install(void)
{
	int i, cc;

	/* printk(KERN_ERR PFX "%s\n", gendata);	/ * ACET string */
	if ((cc = check_module_params()) != 0)
		return -1;

	for (i = 0; i < lun_num; i++) {
		if (install_device(&devices[i], i) == 0)
			continue;
		printk(KERN_ERR PFX
			"ERROR: lun %d not installed, omitting\n",
			devices[i].lun);
		continue;
	}

	/* Register driver */
	cc = register_chrdev(0, DRIVER_NAME, &ctc_fops);
	if (cc < 0) {
		printk(PFX "could not register chardev, exiting [err=%d]\n", cc);
		return cc;
	}
	ctc_major = cc;
	return 0;
}

static void uninstall(void)
{
	int i;

	for (i = 0; i < lun_num; i++) {
		unregister_module(&devices[i]);
	}
	unregister_chrdev(ctc_major, DRIVER_NAME);
}

module_init(install);
module_exit(uninstall);

MODULE_AUTHOR("Juan David Gonzalez Cobas");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CTC OASIS counter driver");
MODULE_SUPPORTED_DEVICE("CTC");

