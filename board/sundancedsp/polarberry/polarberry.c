#include <common.h>
#include <dm.h>
#include <env.h>
#include <init.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define MPFS_SYSREG_SOFT_RESET		((unsigned int *)0x20002088)
#define MPFS_SYS_SERVICE_CR		((unsigned int *)0x37020050)
#define MPFS_SYS_SERVICE_SR		((unsigned int *)0x37020054)
#define MPFS_SYS_SERVICE_MAILBOX_U8	((unsigned char *)0x37020800)
#define MPFS_SYS_SERVICE_MAILBOX_U32	((unsigned int *)0x37020800)

#define SERVICE_CR_REQ_MASK		0x1u
#define SERVICE_SR_BUSY_MASK		0x2u
#define SERVICE_SR_STATUS_SHIFT		16
#define SERVICE_CR_COMMAND_SHIFT	16

#define SYS_SPI_CMD			0x50
#define SYS_SPI_MAILBOX_DATA_LEN	17
#define SYS_SPI_MAILBOX_SRC_OFFSET	8
#define SYS_SPI_MAILBOX_LENGTH_OFFSET	12
#define SYS_SPI_MAILBOX_FREQ_OFFSET	16
#define SYS_SPI_MAILBOX_FREQ		3
#define SPI_FLASH_ADDR			0x400

#define PERIPH_RESET_VALUE		0x800001e8u

/* Descriptor table */
#define START_OFFSET			4
#define END_OFFSET			8
#define SIZE_OFFSET			12
#define DESC_NEXT			12
#define DESC_RESERVED_SIZE		0
#define DESC_SIZE			16

#define BYTES_2				2
#define BYTES_4				4
#define BYTES_8				8
#define BYTES_16			16
#define BYTES_24			24
#define MASK_8BIT			0xff

#define DESIGN_MAGIC_0			0x4d /* 'M' */
#define DESIGN_MAGIC_1			0x43 /* 'C'*/
#define DESIGN_MAGIC_2			0x48 /* 'H'*/
#define DESIGN_MAGIC_3			0x50 /* 'P'*/

static u8 no_of_dtbo;
static u32 dtbos_size;

static void read_device_serial_number(u8 *response, u8 response_size)
{
	u8 idx;
	u8 *response_buf;
	unsigned int val;

	response_buf = (u8 *)response;

	writel(SERVICE_CR_REQ_MASK, MPFS_SYS_SERVICE_CR);
	/*
	 * REQ bit will remain set till the system controller starts
	 * processing.
	 */
	do {
		val = readl(MPFS_SYS_SERVICE_CR);
	} while (SERVICE_CR_REQ_MASK == (val & SERVICE_CR_REQ_MASK));

	/*
	 * Once system controller starts processing the busy bit will
	 * go high and service is completed when busy bit is gone low
	 */
	do {
		val = readl(MPFS_SYS_SERVICE_SR);
	} while (SERVICE_SR_BUSY_MASK == (val & SERVICE_SR_BUSY_MASK));

	for (idx = 0; idx < response_size; idx++)
		response_buf[idx] = readb(MPFS_SYS_SERVICE_MAILBOX_U8 + idx);
}

static u16 execute_sys_service(u8 cmd_opcode, u8 *cmd_data, u16 cmd_data_size)
{
	u32 *word_buf;
	u32 mailbox_val;
	u32 idx, value;
	u16 status;
	u8 *byte_buf;
	u8 byte_offset;
	u8 byte_idx;

	word_buf = (u32 *)cmd_data;

	for (idx = 0; idx < cmd_data_size / BYTES_4; idx++)
		writel(word_buf[idx], MPFS_SYS_SERVICE_MAILBOX_U32 + idx);

	if (cmd_data_size % BYTES_4 > 0) {
		byte_offset = (cmd_data_size / BYTES_4) * BYTES_4;
		byte_buf = (u8 *)(cmd_data + byte_offset);

		mailbox_val = readl(MPFS_SYS_SERVICE_MAILBOX_U32 + idx);

		for (byte_idx = 0; byte_idx < cmd_data_size % 4; byte_idx++) {
			mailbox_val &= ~(MASK_8BIT << (byte_idx * BYTES_8));
			value = byte_buf[byte_idx] << (byte_idx * BYTES_8);
			mailbox_val |= value;
		}
		writel(mailbox_val, MPFS_SYS_SERVICE_MAILBOX_U32 + idx);
	}

	writel((cmd_opcode << SERVICE_CR_COMMAND_SHIFT) | SERVICE_CR_REQ_MASK,
	       MPFS_SYS_SERVICE_CR);

	/*
	 * REQ bit will remain set till the system controller starts
	 * processing.
	 */
	do {
		value = readl(MPFS_SYS_SERVICE_CR);
	} while (SERVICE_CR_REQ_MASK == (value & SERVICE_CR_REQ_MASK));

	/*
	 * Once system controller starts processing the busy bit will
	 * go high and service is completed when busy bit is gone low
	 */
	do {
		value = readl(MPFS_SYS_SERVICE_SR);
	} while (SERVICE_SR_BUSY_MASK == (value & SERVICE_SR_BUSY_MASK));

	status = value >> SERVICE_SR_STATUS_SHIFT;

	return status;
}

static u16 sys_service_spi_copy(void *dst_addr, u32 src_addr, u32 length)
{
	u16 status;
	u8 mailbox_format[SYS_SPI_MAILBOX_DATA_LEN];

	*(u64 *)mailbox_format = (u64)dst_addr;
	*(u32 *)(mailbox_format + SYS_SPI_MAILBOX_SRC_OFFSET) = src_addr;
	*(u32 *)(mailbox_format + SYS_SPI_MAILBOX_LENGTH_OFFSET) = length;
	mailbox_format[SYS_SPI_MAILBOX_FREQ_OFFSET] = SYS_SPI_MAILBOX_FREQ;

	status = execute_sys_service(SYS_SPI_CMD, mailbox_format,
				     SYS_SPI_MAILBOX_DATA_LEN);
	return status;
}

static u16 get_dtbo_desc_header(u8 *desc_data, u32 desc_addr)
{
	u32 length, no_of_descs;
	u16 status;

	/* Get first four bytes to calculate length */
	status = sys_service_spi_copy(desc_data, desc_addr, BYTES_4);
	if (!status) {
		/* Number of descriptors in dtbo descriptor */
		no_of_descs = *((u32 *)desc_data);
		if (no_of_descs) {
			length = DESC_SIZE + ((no_of_descs - 1) * DESC_SIZE);
			status = sys_service_spi_copy(desc_data, desc_addr,
						      length);
		} else {
			status = -1;
		}
	}

	return status;
}

static u8 *get_dtbo(u32 start_addr, u32 size)
{
	u16 status;
	u8 *dtbo;

	dtbo = (u8 *)malloc(size);
	/* Get a dtbo from the spi flash */
	status = sys_service_spi_copy(dtbo, start_addr + SPI_FLASH_ADDR,
				      size);
	if (status) {
		free(dtbo);
		dtbo = NULL;
	}

	return dtbo;
}

static void parse_desc_header(u8 *desc_header)
{
	u32 dtbo_desc_start_addr;
	u32 dtbo_desc_end_addr;
	u32 dtbo_desc_size;
	u32 no_of_descs;
	u16 idx, rsvd = 0;
	u8 dtbo_name[16];
	u8 dtbo_addr[20];
	u8 *desc;
	u8 *dtbo;

	no_of_descs = *((u32 *)desc_header);

	for (idx = 0; idx < no_of_descs; idx++) {
		desc = &desc_header[START_OFFSET + (DESC_NEXT * idx) + rsvd];
		dtbo_desc_start_addr = *((u32 *)desc);

		desc = &desc_header[END_OFFSET + (DESC_NEXT * idx) + rsvd];
		dtbo_desc_end_addr = *((u32 *)desc);

		desc = &desc_header[SIZE_OFFSET + (DESC_NEXT * idx) + rsvd];
		dtbo_desc_size = *((u32 *)desc);

		if (no_of_descs)
			rsvd += DESC_RESERVED_SIZE;

		dtbo = get_dtbo(dtbo_desc_start_addr, dtbo_desc_size);
		if (dtbo) {
			sprintf(dtbo_name, "dtbo_image%d", no_of_dtbo);
			sprintf(dtbo_addr, "0x%llx", (u64)dtbo);
			env_set(dtbo_name, dtbo_addr);
			++no_of_dtbo;
			dtbos_size += dtbo_desc_size;
		}
	}
}

static void get_device_tree_overlays(void)
{
	u32 desc_length;
	u32 dtbo_desc_addr;
	u32 dtbo_addr[5];
	u16 i, status, hart, no_of_harts;
	u8 design_info_desc[256];
	u8 dtbo_desc_data[256];
	u8 no_of_dtbos[8];
	u8 dtbo_size[8];
	u8 *desc;

	no_of_dtbo = 0;
	dtbos_size = 0;

	/* Read first 10 bytes to verify the descriptor is found or not */
	status = sys_service_spi_copy(design_info_desc, SPI_FLASH_ADDR, 10);

	if (!status && design_info_desc[0] == DESIGN_MAGIC_0 &&
	    design_info_desc[1] == DESIGN_MAGIC_1 &&
	    design_info_desc[2] == DESIGN_MAGIC_2 &&
	    design_info_desc[3] == DESIGN_MAGIC_3) {
		desc_length = *((u32 *)&design_info_desc[4]);
		/* Read Design descriptor */
		status = sys_service_spi_copy(design_info_desc,
					      SPI_FLASH_ADDR, desc_length);
		if (!status) {
			no_of_harts = *((u16 *)&design_info_desc[10]);

			for (hart = 0; hart < no_of_harts; hart++) {
				/* Start address of DTBO descriptor */
				desc = &design_info_desc[(0x4 * hart) + 0xc];

				dtbo_desc_addr = *((u32 *)desc);
				dtbo_addr[hart] = dtbo_desc_addr;

				if (!dtbo_addr[hart])
					continue;

				for (i = 0; i < hart; i++) {
					if (dtbo_addr[hart] == dtbo_addr[i])
						continue;
				}

				if (hart && hart == i)
					continue;

				dtbo_desc_addr += SPI_FLASH_ADDR;
				status = get_dtbo_desc_header(dtbo_desc_data,
							      dtbo_desc_addr);
				if (status)
					continue;
				else
					parse_desc_header(dtbo_desc_data);
			}
		}
	}
	sprintf(no_of_dtbos, "%d", no_of_dtbo);
	env_set("no_of_overlays", no_of_dtbos);
	sprintf(dtbo_size, "%d", dtbos_size);
	env_set("dtbo_size", dtbo_size);
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
	mac_addr[0] = 0x00;
	mac_addr[1] = 0x04;
	mac_addr[2] = 0xA3;
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

	get_device_tree_overlays();

	return 0;
}
