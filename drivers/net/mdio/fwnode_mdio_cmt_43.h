/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __FWNODE_MDIO_CMT_43_H__
#define __FWNODE_MDIO_CMT_43_H__

#include <linux/kconfig.h>
#include <linux/stddef.h>

struct fwnode_handle;
struct gpio_desc;
struct mii_bus;

#if IS_ENABLED(CONFIG_TI_DAVINCI_MDIO_CM_T43)
struct gpio_desc *fwnode_mdio_cmt_43_phy_reset(struct mii_bus *bus,
					       struct fwnode_handle *child);
#else
static inline struct gpio_desc *fwnode_mdio_cmt_43_phy_reset(struct mii_bus *bus,
							     struct fwnode_handle *child)
{
	return NULL;
}
#endif

#endif /* __FWNODE_MDIO_CMT_43_H__ */
