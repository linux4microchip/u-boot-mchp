// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Microchip Technology Inc.
 */

#include <common.h>
#include <dm.h>
#include <dm/devres.h>
#include <env.h>
#include <net.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/compat.h>
#include <mpfs-mailbox.h>

DECLARE_GLOBAL_DATA_PTR;

#define PIC64GX_SYSREG_SOFT_RESET         ((unsigned int *)0x20002088)
#define PERIPH_RESET_VALUE		0x800001e8u

int board_init(void)
{
	/* For now nothing to do here. */

	return 0;
}

int board_early_init_f(void)
{
	unsigned int val;

	/* Reset uart, mmc peripheral */
	val = readl(PIC64GX_SYSREG_SOFT_RESET);
	val = (val & ~(PERIPH_RESET_VALUE));
	writel(val, PIC64GX_SYSREG_SOFT_RESET);

	return 0;
}

int board_late_init(void)
{
	u32 ret;
	u32 node_off;
	u8 idx;
	u8 device_serial_number[16] = { 0 };
	ofnode node;
	const u8 *initial_mac_addr;
	unsigned char updated_mac_addr[ARP_HLEN];
	char pic64gx_curiosity_kit_mac_addr[20];
	void *blob = (void *)gd->fdt_blob;
	struct udevice *dev;
	struct mpfs_sys_serv *sys_serv_priv;

	node_off = fdt_path_offset(blob, "ethernet0");
	if (node_off < 0) {
		printf("No ethernet0 path offset\n");
		return -ENODEV;
	}

	node = offset_to_ofnode(node_off);
	initial_mac_addr = ofnode_read_u8_array_ptr(node, "local-mac-address",
						    ARP_HLEN);
	if (!initial_mac_addr) {
		printf("No local-mac-address property\n");
		return -EINVAL;
	}

	memcpy(updated_mac_addr, initial_mac_addr, ARP_HLEN);

	sys_serv_priv = devm_kzalloc(dev, sizeof(*sys_serv_priv), GFP_KERNEL);
	if (!sys_serv_priv)
		return -ENOMEM;

	ret = uclass_get_device_by_name(UCLASS_MISC, "syscontroller", &dev);
	if (ret) {
		debug("%s: system controller setup failed\n", __func__);
		return ret;
	}

	sys_serv_priv->dev = dev;

	sys_serv_priv->sys_controller = mpfs_syscontroller_get(dev);
	ret = IS_ERR(sys_serv_priv->sys_controller);
	if (ret) {
		debug("%s:  Failed to register system controller sub device ret=%d\n", __func__, ret);
		return -ENODEV;
	}

	ret = mpfs_syscontroller_read_sernum(sys_serv_priv, device_serial_number);
	if (ret) {
		printf("Cannot read device serial number\n");
		return -EINVAL;
	}

	/* Update MAC address with device serial number */
	updated_mac_addr[0] = 0x00;
	updated_mac_addr[1] = 0x04;
	updated_mac_addr[2] = 0xA3;
	updated_mac_addr[3] = device_serial_number[2];
	updated_mac_addr[4] = device_serial_number[1];
	updated_mac_addr[5] = device_serial_number[0];

	ret = ofnode_write_prop(node, "local-mac-address", updated_mac_addr,
				ARP_HLEN, true);
	if (ret) {
		printf("Error setting local-mac-address property\n");
		return -ENODEV;
	}
	pic64gx_curiosity_kit_mac_addr[0] = '[';

	sprintf(&pic64gx_curiosity_kit_mac_addr[1], "%pM", updated_mac_addr);

	pic64gx_curiosity_kit_mac_addr[18] = ']';
	pic64gx_curiosity_kit_mac_addr[19] = '\0';

	for (idx = 0; idx < 20; idx++) {
		if (pic64gx_curiosity_kit_mac_addr[idx] == ':')
			pic64gx_curiosity_kit_mac_addr[idx] = ' ';
	}
	env_set("pic64gx_curiosity_kit_mac_addr0", pic64gx_curiosity_kit_mac_addr);

	mpfs_syscontroller_process_dtbo(sys_serv_priv);

	return 0;
}
