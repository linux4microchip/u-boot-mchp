// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Microchip Technology Inc.
 * Padmarao Begari <padmarao.begari@microchip.com>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <env.h>
#include <init.h>
#include <mmc.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define MPFS_SYSREG_SOFT_RESET		((unsigned int *)0x20002088)
#define MPFS_SYS_SERVICE_CR		((unsigned int *)0x37020050)
#define MPFS_SYS_SERVICE_SR		((unsigned int *)0x37020054)
#define MPFS_SYS_SERVICE_MAILBOX	((unsigned char *)0x37020800)

#define GPIO_IOBANK0_LO_GPOUT		((unsigned char *)0x20120088)
#define GPIO_IOBANK0_LO_CLEAR_BITS	((unsigned char *)0x201200a0)
#define GPIO_IOBANK0_LO_SET_BITS	((unsigned char *)0x201200a4)
#define GPIO_IOBANK0_SD_SEL_MASK	0x1000 /* bit #12 selects the SD card */

#define MPFS_MMC_DRIVER_PATH		"/soc/mmc@20008000"
#define MPFS_MMC_DRIVER_NAME		"sdhci-cdns"
#define MPFS_MMC_DEV_NUM		0

#define PERIPH_RESET_VALUE		0x1e8u
#define SERVICE_CR_REQ			0x1u
#define SERVICE_SR_BUSY			0x2u

static void read_device_serial_number(u8 *response, u8 response_size)
{
	u8 idx;
	u8 *response_buf;
	unsigned int val;

	response_buf = (u8 *)response;

	writel(SERVICE_CR_REQ, MPFS_SYS_SERVICE_CR);
	/*
	 * REQ bit will remain set till the system controller starts
	 * processing.
	 */
	do {
		val = readl(MPFS_SYS_SERVICE_CR);
	} while (SERVICE_CR_REQ == (val & SERVICE_CR_REQ));

	/*
	 * Once system controller starts processing the busy bit will
	 * go high and service is completed when busy bit is gone low
	 */
	do {
		val = readl(MPFS_SYS_SERVICE_SR);
	} while (SERVICE_SR_BUSY == (val & SERVICE_SR_BUSY));

	for (idx = 0; idx < response_size; idx++)
		response_buf[idx] = readb(MPFS_SYS_SERVICE_MAILBOX + idx);
}

int board_init(void)
{
	/* For now nothing to do here. */

	return 0;
}

int board_early_init_f(void)
{
	unsigned int val;

	/* Reset uart, mmc peripheral */
	val = readl(MPFS_SYSREG_SOFT_RESET);
	val = (val & ~(PERIPH_RESET_VALUE));
	writel(val, MPFS_SYSREG_SOFT_RESET);
	return 0;
}

int board_late_init(void)
{
	u32 ret;
	u32 node;
	u8 idx;
	u8 device_serial_number[16] = { 0 };
	unsigned char mac_addr[6];
	char icicle_mac_addr[20];
	void *blob = (void *)gd->fdt_blob;

	node = fdt_path_offset(blob, "ethernet0");
	if (node < 0) {
		printf("No ethernet0 path offset\n");
		return -ENODEV;
	}

	ret = fdtdec_get_byte_array(blob, node, "local-mac-address", mac_addr, 6);
	if (ret) {
		printf("No local-mac-address property\n");
		return -EINVAL;
	}

	read_device_serial_number(device_serial_number, 16);

	/* Update MAC address with device serial number */
	mac_addr[0] = 0xc0;
	mac_addr[1] = 0xe5;
	mac_addr[2] = 0x4e;
	mac_addr[3] = device_serial_number[2];
	mac_addr[4] = device_serial_number[1];
	mac_addr[5] = device_serial_number[0];

	ret = fdt_setprop(blob, node, "local-mac-address", mac_addr, 6);
	if (ret) {
		printf("Error setting local-mac-address property\n");
		return -ENODEV;
	}

	icicle_mac_addr[0] = '[';

	sprintf(&icicle_mac_addr[1], "%pM", mac_addr);

	icicle_mac_addr[18] = ']';
	icicle_mac_addr[19] = '\0';

	for (idx = 0; idx < 20; idx++) {
		if (icicle_mac_addr[idx] == ':')
			icicle_mac_addr[idx] = ' ';
	}
	env_set("icicle_mac_addr0", icicle_mac_addr);

	mac_addr[5] = device_serial_number[0] + 1;

	icicle_mac_addr[0] = '[';

	sprintf(&icicle_mac_addr[1], "%pM", mac_addr);

	icicle_mac_addr[18] = ']';
	icicle_mac_addr[19] = '\0';

	for (idx = 0; idx < 20; idx++) {
		if (icicle_mac_addr[idx] == ':')
			icicle_mac_addr[idx] = ' ';
	}
	env_set("icicle_mac_addr1", icicle_mac_addr);

	return 0;
}

/* code derived from cmd/bind.c */
static int unbind_mmc_dev(void)
{
	const char *path = MPFS_MMC_DRIVER_PATH;
	struct udevice *dev;
	int ret;
	ofnode ofnode;

	ofnode = ofnode_path(path);
	printf("%s: node=%ld\n", __func__, ofnode.of_offset);
	if (!ofnode_valid(ofnode)) {
		printf("%s is not a valid node path\n", path);
		return -EINVAL;
	}

	ret = device_find_global_by_ofnode(ofnode, &dev);

	if (!dev || ret) {
		printf("Cannot find a device with path %s\n", path);
		return -ENODEV;
	}

	ret = device_remove(dev, DM_REMOVE_NORMAL);
	if (ret) {
		printf("Unable to remove. err:%d\n", ret);
		return ret;
	}

	ret = device_unbind(dev);
	if (ret) {
		printf("Unable to unbind. err:%d\n", ret);
		return ret;
	}

	return 0;
}

static int bind_and_init_mmc_dev(void)
{
	const char *path = MPFS_MMC_DRIVER_PATH;
	const char *drv_name = MPFS_MMC_DRIVER_NAME;
	struct udevice *dev;
	struct udevice *parent = NULL;
	int ret;
	ofnode ofnode;
	struct driver *drv;
	drv = lists_driver_lookup_name(drv_name);
	if (!drv) {
		printf("%s is not a valid driver name\n", drv_name);
		return -ENOENT;
	}

	ofnode = ofnode_path(path);
	printf("%s: node=%ld\n", __func__, ofnode.of_offset);
	if (!ofnode_valid(ofnode)) {
		printf("%s is not a valid node path\n", path);
		return -EINVAL;
	}

	while (ofnode_valid(ofnode)) {
		if (!device_find_global_by_ofnode(ofnode, &parent))
			break;
		ofnode = ofnode_get_parent(ofnode);
		printf("%s: parent=%ld\n", __func__, ofnode.of_offset);
	}

	if (!parent) {
		printf("Cannot find a parent device for node path %s\n", path);
		return -ENODEV;
	}

	ofnode = ofnode_path(path);
	printf("%s: node=%ld\n", __func__, ofnode.of_offset);
	ret = lists_bind_fdt(parent, ofnode, &dev, NULL, false);

	if (!dev || ret) {
		printf("Unable to bind. err:%d\n", ret);
		return ret;
	}

	return mmc_init_device(MPFS_MMC_DEV_NUM);
}

static int fdt_mmc_fixup(int sd)
{
	void *blob = (void *)gd->fdt_blob;
	u32 bus_width;
	u32 node;
	int ret;

	node = fdt_path_offset(blob, MPFS_MMC_DRIVER_PATH);
	if (node < 0) {
		printf("No mmc path offset\n");
		return -ENODEV;
	}
	printf("%s: node=%d\n", __func__, node);

	bus_width = sd ? 4 : 8;
	ret = fdt_setprop_u32(blob, node, "bus-width", bus_width);
	if (ret) {
		printf("Error setting bus-width property\n");
		return -ENODEV;
	}

	return 0;
}

int do_selsd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned int val;
	int sel = -1;
	int ret;

	val = readl(GPIO_IOBANK0_LO_GPOUT);
        if (argc == 1) {
		if (val & GPIO_IOBANK0_SD_SEL_MASK)
			return 0;
		else
		return 1;
        } else if (argc == 2) {
		if (argv[1][0] == '0') {
			if (val & GPIO_IOBANK0_SD_SEL_MASK)
				sel = 0;
		} else if (argv[1][0] == '1') {
			if (!(val & GPIO_IOBANK0_SD_SEL_MASK))
				sel = 1;
		} else {
			return CMD_RET_USAGE;
		}
		if (sel >= 0) {
			ret = unbind_mmc_dev();
			if (ret)
				return CMD_RET_FAILURE;
			if (sel) {
				writel(GPIO_IOBANK0_SD_SEL_MASK,
				       GPIO_IOBANK0_LO_SET_BITS);
			} else {
				writel(GPIO_IOBANK0_SD_SEL_MASK,
				       GPIO_IOBANK0_LO_CLEAR_BITS);
			}
			ret = fdt_mmc_fixup(sel);
			if (ret)
				return CMD_RET_FAILURE;			
			ret = bind_and_init_mmc_dev();
			if (ret)
				return CMD_RET_FAILURE;
		}
	} else {
		return CMD_RET_USAGE;
	}

        return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
        selsd, CONFIG_SYS_MAXARGS, 1, do_selsd,
        "Get or set the SD card as MMC device\n",
        "selsd - returns 0 if the SD card or 1 if the eMMC is selected\n"
        "selsd 1 - select the SD card\n"
        "selsd 0 - select the eMMC\n"
        );
