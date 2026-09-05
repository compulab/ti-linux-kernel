// SPDX-License-Identifier: GPL-2.0-only
/* CompuLab CM-T43 PHY reset handling before MDIO discovery. */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/property.h>

#include "fwnode_mdio_cmt_43.h"

#define CMT_43_PHY_INIT_PROPERTY "compulab,cm-t43-phy-init"

struct gpio_desc *fwnode_mdio_cmt_43_phy_reset(struct mii_bus *bus,
					       struct fwnode_handle *child)
{
	struct fwnode_handle *bus_fwnode = dev_fwnode(&bus->dev);
	struct gpio_desc *reset_gpio;
	u32 assert_delay = 0;
	u32 deassert_delay = 0;

	if (!bus_fwnode ||
	    !fwnode_property_read_bool(bus_fwnode, CMT_43_PHY_INIT_PROPERTY))
		return NULL;

	if (!IS_ENABLED(CONFIG_GPIOLIB))
		return NULL;

	reset_gpio = fwnode_gpiod_get_index(child, "reset", 0,
					    GPIOD_OUT_HIGH, "PHY reset");
	if (IS_ERR(reset_gpio)) {
		if (PTR_ERR(reset_gpio) == -ENOENT)
			return NULL;

		return reset_gpio;
	}

	fwnode_property_read_u32(child, "reset-assert-us", &assert_delay);
	fwnode_property_read_u32(child, "reset-deassert-us", &deassert_delay);

	if (assert_delay)
		fsleep(assert_delay);

	gpiod_set_value_cansleep(reset_gpio, 0);

	if (deassert_delay)
		fsleep(deassert_delay);

	return reset_gpio;
}
EXPORT_SYMBOL_GPL(fwnode_mdio_cmt_43_phy_reset);

MODULE_DESCRIPTION("CompuLab CM-T43 pre-discovery PHY reset");
MODULE_LICENSE("GPL");
