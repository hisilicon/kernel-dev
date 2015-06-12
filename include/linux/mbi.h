#ifndef _LINUX_MBI_H
#define _LINUX_MBI_H

#include <linux/device.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>

struct mbi_ops;

/**
 * struct mbi_msg - MBI message descriptor
 *
 * @address_lo:	lower 32bit value of MBI address register
 * @address_hi:	higher 32bit value of MBI address register
 * @data:	data value of MBI data register
 */
struct mbi_msg {
	u32	address_lo;
	u32	address_hi;
	u32	data;
};

/**
 * struct mbi_desc - Message Based Interrupt (MBI) descriptor
 *
 * @dev:	the device owned the MBI
 * @ops:	config operations of @dev
 * @msg_id:	identifier of the message group
 * @lines:	max number of interrupts supported by the message register
 * @irq:	base linux interrupt number of the MBI
 * @nvec:	number of interrupts controlled by the MBI
 * @offset:	hardware pin offset of @irq
 * @data:	message specific data
 */
struct mbi_desc {
	/* MBI-related device information */
	struct {
		struct device	*dev;
		struct mbi_ops	*ops;
		int		msg_id;
		unsigned int	lines;
	} mbi;
	/* Message properties */
	unsigned int		irq;
	unsigned int		nvec;
	int			offset;
	void			*data;
};

/**
 * struct mbi_ops - MBI functions of MBI-capable device
 *
 * @write_msg:	write message registers for an MBI
 * @mask_irq:	mask an MBI interrupt
 * @unmask_irq:	unmask an MBI interrupt
 */
struct mbi_ops {
	void	(*write_msg)(struct mbi_desc *desc, struct mbi_msg *msg, bool enable);
	void	(*mask_irq)(struct mbi_desc *desc);
	void	(*unmask_irq)(struct mbi_desc *desc);
};

/* Functions to allocate an MBI descriptor */
struct mbi_desc *mbi_alloc_desc(struct device *dev, struct mbi_ops *ops,
				int msg_id, unsigned int lines,
				unsigned int offset, void *data);

/* Create hierarchy MBI domain for interrupt controllers */
struct irq_domain *mbi_create_irq_domain(struct device_node *np,
					 struct irq_domain *parent, void *arg);

/* Function to parse and map message interrupts */
int mbi_parse_irqs(struct device *dev, struct mbi_ops *ops);
void mbi_free_irqs(struct device *dev, unsigned int virq, unsigned int nvec);
#endif /* _LINUX_MBI_H */
