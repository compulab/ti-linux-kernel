// SPDX-License-Identifier: GPL-2.0
/* CompuLab CM-T43 Ethernet PHY reference clock initialization. */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>

#include "davinci_mdio_cm_t43.h"

static void davinci_mdio_cm_t43_disable_phy_clk(void *data)
{
	clk_disable_unprepare(data);
}

int davinci_mdio_cm_t43_enable_phys(struct device *dev)
{
	struct clk *phy_clk;
	int ret;

	phy_clk = devm_clk_get(dev, "phy-ref");
	if (IS_ERR(phy_clk))
		return dev_err_probe(dev, PTR_ERR(phy_clk),
				     "failed to get CM-T43 PHY reference clock\n");

	ret = clk_prepare_enable(phy_clk);
	if (ret)
		return dev_err_probe(dev, ret,
				     "failed to enable CM-T43 PHY reference clock\n");

	ret = devm_add_action_or_reset(dev,
				       davinci_mdio_cm_t43_disable_phy_clk,
				       phy_clk);
	if (ret)
		return ret;

	fsleep(10000);

	dev_info(dev, "CM-T43 PHY reference clock enabled at %lu Hz\n",
		 clk_get_rate(phy_clk));

	return 0;
}
EXPORT_SYMBOL_GPL(davinci_mdio_cm_t43_enable_phys);

MODULE_DESCRIPTION("CompuLab CM-T43 Ethernet PHY initialization");
MODULE_LICENSE("GPL");
