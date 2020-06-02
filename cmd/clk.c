// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Xilinx, Inc.
 */
#include <common.h>
#include <command.h>
#include <clk.h>
#if defined(CONFIG_DM) && defined(CONFIG_CLK)
#include <dm.h>
#include <dm/device.h>
#include <dm/root.h>
#include <dm/device-internal.h>
#include <linux/clk-provider.h>
#endif

#if defined(CONFIG_DM) && defined(CONFIG_CLK)
static void show_clks(struct udevice *dev, int depth, int last_flag)
{
	int i, is_last;
	struct udevice *child;
	struct clk *clkp;
	u32 rate;

	clkp = dev_get_clk_ptr(dev);
	if (device_get_uclass_id(dev) == UCLASS_CLK && clkp) {
		rate = clk_get_rate(clkp);

	printf(" %-12u  %8d        ", rate, clkp->enable_count);

	for (i = depth; i >= 0; i--) {
		is_last = (last_flag >> i) & 1;
		if (i) {
			if (is_last)
				printf("    ");
			else
				printf("|   ");
		} else {
			if (is_last)
				printf("`-- ");
			else
				printf("|-- ");
		}
	}

	printf("%s\n", dev->name);
	}

	list_for_each_entry(child, &dev->child_head, sibling_node) {
		is_last = list_is_last(&child->sibling_node, &dev->child_head);
		show_clks(child, depth + 1, (last_flag << 1) | is_last);
	}
}

int __weak soc_clk_dump(void)
{
	struct udevice *root;

	root = dm_root();
	if (root) {
		printf(" Rate               Usecnt      Name\n");
		printf("------------------------------------------\n");
		show_clks(root, -1, 0);
	}

	return 0;
}
#else
int __weak soc_clk_dump(void)
{
	puts("Not implemented\n");
	return 1;
}
#endif

static int do_clk_dump(cmd_tbl_t *cmdtp, int flag, int argc,
		       char *const argv[])
{
	int ret;

	ret = soc_clk_dump();
	if (ret < 0) {
		printf("Clock dump error %d\n", ret);
		ret = CMD_RET_FAILURE;
	}

	return ret;
}

static cmd_tbl_t cmd_clk_sub[] = {
	U_BOOT_CMD_MKENT(dump, 1, 1, do_clk_dump, "", ""),
};

static int do_clk(cmd_tbl_t *cmdtp, int flag, int argc,
		  char *const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* Strip off leading 'clk' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_clk_sub[0], ARRAY_SIZE(cmd_clk_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

static int do_clk_enable(cmd_tbl_t *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct clk *clk;
	unsigned int clkid;
	long ret;

	if (!argv[1])
		return -EINVAL;

	clkid = simple_strtoul(argv[1], NULL, 10);

	ret = clk_get_by_id(clkid, &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(clk);

	return ret;
}

static int do_clk_disable(cmd_tbl_t *cmdtp, int flag, int argc,
			  char *const argv[])
{
	struct clk *clk;
	unsigned int clkid;
	long ret;

	if (!argv[1])
		return -EINVAL;

	clkid = simple_strtoul(argv[1], NULL, 10);

	ret = clk_get_by_id(clkid, &clk);
	if (ret < 0)
		return ret;

	ret = clk_disable(clk);

	return ret;
}

static int do_clk_status(cmd_tbl_t *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct clk *clk;
	unsigned int clkid;
	long ret;

	if (!argv[1])
		return -EINVAL;

	clkid = simple_strtoul(argv[1], NULL, 10);

	ret = clk_get_by_id(clkid, &clk);
	if (ret < 0)
		return ret;

	printf("enable: %d\n", clk->enable_count);

	return 0;
}

static int do_clk_get_rate(cmd_tbl_t *cmdtp, int flag, int argc,
			   char *const argv[])
{
	struct clk *clk;
	unsigned int clkid;
	long ret;

	if (!argv[1])
		return -EINVAL;

	clkid = simple_strtoul(argv[1], NULL, 10);

	ret = clk_get_by_id(clkid, &clk);
	if (ret < 0)
		return ret;

	ret = clk_get_rate(clk);
	if (ret < 0)
		return ret;

	printf("rate: %lu\n", (ulong)ret);

	return ret;
}

static int do_clk_set_rate(cmd_tbl_t *cmdtp, int flag, int argc,
			   char *const argv[])
{
	struct clk *clk;
	unsigned int clkid;
	unsigned long rate;
	long ret = 0;

	if (!argv[1] || !argv[2])
		return -EINVAL;

	clkid = simple_strtoul(argv[1], NULL, 10);
	rate = simple_strtoul(argv[2], NULL, 10);

	ret = clk_get_by_id(clkid, &clk);
	if (ret < 0)
		return ret;

	ret = clk_set_rate(clk, rate);

	return ret;
}

static int do_clk_get_parent(cmd_tbl_t *cmdtp, int flag, int argc,
			     char *const argv[])
{
	struct clk *clk, *pclk;
	unsigned int clkid;
	int ret;

	if (!argv[1])
		return -EINVAL;

	clkid = simple_strtoul(argv[1], NULL, 10);

	ret = clk_get_by_id(clkid, &clk);
	if (ret < 0)
		return ret;

	pclk = clk_get_parent(clk);
	if (IS_ERR(pclk))
		return PTR_ERR(pclk);

	printf("clkid: %d, name: %s\n", clkid, clk_hw_get_name(pclk));

	return 0;
}

static int do_clk_set_parent(cmd_tbl_t *cmdtp, int flag, int argc,
			     char *const argv[])
{
	struct clk *clk, *pclk;
	unsigned int clkid;
	unsigned int pclkid;
	int ret;

	if (!argv[1] || !argv[2])
		return -EINVAL;

	clkid = simple_strtoul(argv[1], NULL, 10);
	pclkid = simple_strtoul(argv[2], NULL, 10);

	ret = clk_get_by_id(clkid, &clk);
	if (ret < 0)
		return ret;

	ret = clk_get_by_id(pclkid, &pclk);
	if (ret < 0)
		return ret;

	ret = clk_set_parent(clk, pclk);
	if (ret < 0)
		return ret;

	return 0;
}

static cmd_tbl_t cmd_clk_test_sub[] = {
	U_BOOT_CMD_MKENT(ena, 1, 1, do_clk_enable, "", ""),
	U_BOOT_CMD_MKENT(dis, 1, 1, do_clk_disable, "", ""),
	U_BOOT_CMD_MKENT(status, 1, 1, do_clk_status, "", ""),
	U_BOOT_CMD_MKENT(grate, 1, 1, do_clk_get_rate, "", ""),
	U_BOOT_CMD_MKENT(srate, 2, 1, do_clk_set_rate, "", ""),
	U_BOOT_CMD_MKENT(gparent, 1, 1, do_clk_get_parent, "", ""),
	U_BOOT_CMD_MKENT(sparent, 2, 1, do_clk_set_parent, "", ""),
};

static int clk_test(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_clk_test_sub[0],
			 ARRAY_SIZE(cmd_clk_test_sub));
	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

#ifdef CONFIG_SYS_LONGHELP
static char clk_help_text[] =
	"dump - Print clock frequencies";

static char clk_test_help_text[] =
	"\n"
	"ena - enable clock\n"
	"dis - disable clock\n"
	"status - get clock status\n"
	"grate - get clock rate\n"
	"srate - set clock rate\n"
	"gparent - get clock parent\n"
	"sparent - set clock parent\n";
#endif

U_BOOT_CMD(clk, 2, 1, do_clk, "CLK sub-system", clk_help_text);
U_BOOT_CMD(clktest, 4, 1, clk_test, "Test clock", clk_test_help_text);
