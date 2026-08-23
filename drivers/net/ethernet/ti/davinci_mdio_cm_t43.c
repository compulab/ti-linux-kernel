// SPDX-License-Identifier: GPL-2.0
/* CompuLab CM-T43 Ethernet PHY clock and reset initialization. */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio/consumer.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>

#include "davinci_mdio_cm_t43.h"

#define CM_T43_CTRL_BASE	0x44e10000
#define CM_T43_CTRL_SIZE	0x1000
#define CM_T43_CLKOUT2_PAD	0x0a74
#define CM_T43_PRCM_BASE	0x44df0000
#define CM_T43_PRCM_SIZE	0x10000
#define CM_T43_DPLL_EXTDEV_MODE	0x2e60
#define CM_T43_DPLL_EXTDEV_CLKSEL	0x2e6c
#define CM_T43_DPLL_EXTDEV_M2	0x2e70
#define CM_T43_CLKOUT2_CTRL	0x4108

int davinci_mdio_cm_t43_enable_phys(struct device *dev)
{
	struct gpio_desc *reset_gpios[2] = { NULL, NULL };
	struct device_node *child;
	void __iomem *ctrl_base;
	void __iomem *prcm_base;
	int count = 0;
	int ret = 0;
	int i;

	ctrl_base = ioremap(CM_T43_CTRL_BASE, CM_T43_CTRL_SIZE);
	prcm_base = ioremap(CM_T43_PRCM_BASE, CM_T43_PRCM_SIZE);
	if (!ctrl_base || !prcm_base) {
		dev_err(dev, "failed to map CM-T43 PHY clock registers\n");
		ret = -ENOMEM;
		goto unmap;
	}

	/* Restore the PHY reference clock configuration used by U-Boot. */
	writel(0x00040003, ctrl_base + CM_T43_CLKOUT2_PAD);
	writel(0x00010006, prcm_base + CM_T43_CLKOUT2_CTRL);
	writel(0x00000004, prcm_base + CM_T43_DPLL_EXTDEV_MODE);
	writel(0x02000009, prcm_base + CM_T43_DPLL_EXTDEV_CLKSEL);
	writel(0x0000038a, prcm_base + CM_T43_DPLL_EXTDEV_M2);
	readl(prcm_base + CM_T43_CLKOUT2_CTRL);
	fsleep(10000);

	for_each_available_child_of_node(dev->of_node, child) {
		struct gpio_desc *gpio;

		if (count == ARRAY_SIZE(reset_gpios))
			continue;

		gpio = gpiod_get_from_of_node(child, "reset-gpios",
					      0, GPIOD_OUT_HIGH,
					      "CM-T43 PHY reset");
		if (IS_ERR(gpio)) {
			ret = dev_err_probe(dev, PTR_ERR(gpio),
					    "failed to assert PHY reset\n");
			of_node_put(child);
			goto release_gpios;
		}
		if (gpio)
			reset_gpios[count++] = gpio;
	}

	if (count != ARRAY_SIZE(reset_gpios)) {
		dev_err(dev, "expected two CM-T43 PHY reset GPIOs, found %d\n",
			count);
		ret = -ENODEV;
		goto release_gpios;
	}

	msleep(20);
	for (i = 0; i < count; i++)
		gpiod_set_value_cansleep(reset_gpios[i], 0);
	msleep(100);

	dev_info(dev,
		 "CM-T43 PHY clock/reset initialized: clkout2=%08x dpll=%08x pad=%08x\n",
		 readl(prcm_base + CM_T43_CLKOUT2_CTRL),
		 readl(prcm_base + CM_T43_DPLL_EXTDEV_MODE),
		 readl(ctrl_base + CM_T43_CLKOUT2_PAD));

release_gpios:
	for (i = 0; i < count; i++)
		gpiod_put(reset_gpios[i]);
unmap:
	if (prcm_base)
		iounmap(prcm_base);
	if (ctrl_base)
		iounmap(ctrl_base);

	return ret;
}
EXPORT_SYMBOL_GPL(davinci_mdio_cm_t43_enable_phys);

MODULE_DESCRIPTION("CompuLab CM-T43 Ethernet PHY initialization");
MODULE_LICENSE("GPL");
