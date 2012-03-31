/*
 * Copyright (C) 2005-2006 Atmel Corporation
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef __DRIVERS_GMACB_H__
#define __DRIVERS_GMACB_H__

/* MACB register offsets */
#define GMACB_NCR				0x0000
#define GMACB_NCFGR				0x0004
#define GMACB_NSR				0x0008
#define	GMACB_UR				0x000c
#define GMACB_DCFGR				0x0010
#define GMACB_TSR				0x0014
#define GMACB_RBQB				0x0018
#define GMACB_TBQB				0x001c
#define GMACB_RSR				0x0020
#define GMACB_ISR				0x0024
#define GMACB_IER				0x0028
#define GMACB_IDR				0x002c
#define GMACB_IMR				0x0030
#define GMACB_MAN				0x0034
#define GMACB_RPQ				0x0038
#define GMACB_TPQ				0x003c
#define GMACB_TPSF				0x0040
#define GMACB_RPSF				0x0044
#define GMACB_HRB				0x0080
#define GMACB_HRT				0x0084
#define GMACB_SA1B				0x0088
#define GMACB_SA1T				0x008c
#define GMACB_SA2B				0x0090
#define GMACB_SA2T				0x0094
#define GMACB_SA3B				0x0098
#define GMACB_SA3T				0x009c
#define GMACB_SA4B				0x00a0
#define GMACB_SA4T				0x00a4
#define GMACB_TIDM1				0x00a8
#define GMACB_TIDM2				0x00ac
#define GMACB_TIDM3				0x00b0
#define GMACB_TIDM4				0x00b4
#define GMACB_WOL				0x00b8
#define GMACB_IPGS				0x00bc
#define GMACB_SVLAN				0x00c0
#define GMACB_TPFCP				0x00c4
#define GMACB_SAMB1				0x00c8
#define GMACB_SAMT1				0x00cc

#define GMACB_FT				0x0108
#define GMACB_BCFT				0x010c




	/* Bitfields in NCR */
#define GMACB_LB_OFFSET				0
#define GMACB_LB_SIZE				1
#define GMACB_LBL_OFFSET			1
#define GMACB_LBL_SIZE				1
#define GMACB_RE_OFFSET				2
#define GMACB_RE_SIZE				1
#define GMACB_TE_OFFSET				3
#define GMACB_TE_SIZE				1
#define GMACB_MPE_OFFSET				4
#define GMACB_MPE_SIZE				1
#define GMACB_CLRSTAT_OFFSET			5
#define GMACB_CLRSTAT_SIZE			1
#define GMACB_INCSTAT_OFFSET			6
#define GMACB_INCSTAT_SIZE			1
#define GMACB_WESTAT_OFFSET			7
#define GMACB_WESTAT_SIZE			1
#define GMACB_BP_OFFSET				8
#define GMACB_BP_SIZE				1
#define GMACB_TSTART_OFFSET			9
#define GMACB_TSTART_SIZE			1
#define GMACB_THALT_OFFSET			10
#define GMACB_THALT_SIZE				1
#define GMACB_NCR_TPF_OFFSET			11
#define GMACB_NCR_TPF_SIZE			1
#define GMACB_TZQ_OFFSET				12
#define GMACB_TZQ_SIZE				1

	/* Bitfields in NCFGR */
#define GMACB_SPD_OFFSET			0
#define GMACB_SPD_SIZE				1
#define GMACB_FD_OFFSET				1
#define GMACB_FD_SIZE				1
#define GMACB_DNVLAN_OFFSET			2
#define GMACB_DNVLAN_SIZE			1
#define GMACB_JFRAME_OFFSET			3
#define GMACB_JFRAME_SIZE			1
#define GMACB_CAF_OFFSET			4
#define GMACB_CAF_SIZE				1
#define GMACB_NBC_OFFSET			5
#define GMACB_NBC_SIZE				1
#define GMACB_NCFGR_MTI_OFFSET			6
#define GMACB_NCFGR_MTI_SIZE			1
#define GMACB_UNI_OFFSET			7
#define GMACB_UNI_SIZE				1
#define MACB_MAXFS_OFFSET			8
#define MACB_MAXFS_SIZE				1
#define GMACB_GBE_OFFSET			10
#define GMACB_GBE_SIZE				1
#define GMACB_PIS_OFFSET			11
#define GMACB_PIS_SIZE				1
#define GMACB_RTY_OFFSET				12
#define GMACB_RTY_SIZE				1
#define GMACB_PEN_OFFSET				13
#define GMACB_PEN_SIZE				1
#define GMACB_RXBUFO_OFFSET			14
#define GMACB_RXBUFO_SIZE				2
#define GMACB_LFERD_OFFSET			16
#define GMACB_LFERD_SIZE				1
#define GMACB_RFCS_OFFSET			17
#define GMACB_RFCS_SIZE				1
#define GMACB_CLK_OFFSET			18
#define GMACB_CLK_SIZE				3
#define GMACB_DBW_OFFSET			21
#define GMACB_DBW_SIZE				2

	/* Bitfields in NSR */
#define GMACB_NSR_LINK_OFFSET			0
#define GMACB_NSR_LINK_SIZE			1
#define GMACB_MDIO_OFFSET			1
#define GMACB_MDIO_SIZE				1
#define GMACB_IDLE_OFFSET			2
#define GMACB_IDLE_SIZE				1

	/* Bitfields in TSR */
#define GMACB_UBR_OFFSET				0
#define GMACB_UBR_SIZE				1
#define GMACB_COL_OFFSET				1
#define GMACB_COL_SIZE				1
#define GMACB_TSR_RLE_OFFSET			2
#define GMACB_TSR_RLE_SIZE			1
#define GMACB_TGO_OFFSET				3
#define GMACB_TGO_SIZE				1
#define GMACB_BEX_OFFSET				4
#define GMACB_BEX_SIZE				1
#define GMACB_COMP_OFFSET			5
#define GMACB_COMP_SIZE				1
#define GMACB_UND_OFFSET				6
#define GMACB_UND_SIZE				1

	/* Bitfields in RSR */
#define GMACB_BNA_OFFSET				0
#define GMACB_BNA_SIZE				1
#define GMACB_REC_OFFSET				1
#define GMACB_REC_SIZE				1
#define GMACB_OVR_OFFSET				2
#define GMACB_OVR_SIZE				1

	/* Bitfields in ISR/IER/IDR/IMR */
#define GMACB_MFD_OFFSET				0
#define GMACB_MFD_SIZE				1
#define GMACB_RCOMP_OFFSET			1
#define GMACB_RCOMP_SIZE				1
#define GMACB_RXUBR_OFFSET			2
#define GMACB_RXUBR_SIZE				1
#define GMACB_TXUBR_OFFSET			3
#define GMACB_TXUBR_SIZE				1
#define GMACB_ISR_TUND_OFFSET			4
#define GMACB_ISR_TUND_SIZE			1
#define GMACB_ISR_RLE_OFFSET			5
#define GMACB_ISR_RLE_SIZE			1
#define GMACB_TXERR_OFFSET			6
#define GMACB_TXERR_SIZE				1
#define GMACB_TCOMP_OFFSET			7
#define GMACB_TCOMP_SIZE				1
#define GMACB_ISR_LINK_OFFSET			9
#define GMACB_ISR_LINK_SIZE			1
#define GMACB_ISR_ROVR_OFFSET			10
#define GMACB_ISR_ROVR_SIZE			1
#define GMACB_HRESP_OFFSET			11
#define GMACB_HRESP_SIZE				1
#define GMACB_PFR_OFFSET				12
#define GMACB_PFR_SIZE				1
#define GMACB_PTZ_OFFSET				13
#define GMACB_PTZ_SIZE				1

	/* Bitfields in MAN */
#define GMACB_DATA_OFFSET			0
#define GMACB_DATA_SIZE				16
#define GMACB_WTN_OFFSET			16
#define GMACB_WTN_SIZE				2
#define GMACB_REGA_OFFSET			18
#define GMACB_REGA_SIZE				5
#define GMACB_PHYA_OFFSET			23
#define GMACB_PHYA_SIZE				5
#define GMACB_OP_OFFSET				28
#define GMACB_OP_SIZE				2
#define GMACB_CLTTO_OFFSET				30
#define GMACB_CLTTO_SIZE				1
#define GMACB_WZO_OFFSET				31
#define GMACB_WZO_SIZE				1

	/* Bitfields in US */
#define GMACB_GMII_OFFSET				0
#define GMACB_GMII_SIZE				1

	/* Bitfields in USRIO (AT91) */
#define GMACB_RGMII_OFFSET			0
#define GMACB_RGMII_SIZE				1

	/* Bitfields in WOL */
#define GMACB_IP_OFFSET				0
#define GMACB_IP_SIZE				16
#define GMACB_MAG_OFFSET				16
#define GMACB_MAG_SIZE				1
#define GMACB_ARP_OFFSET				17
#define GMACB_ARP_SIZE				1
#define GMACB_SA1_OFFSET				18
#define GMACB_SA1_SIZE				1
#define GMACB_WOL_MTI_OFFSET			19
#define GMACB_WOL_MTI_SIZE			1

	/* Constants for CLK */
#define GMACB_CLK_DIV8				0
#define GMACB_CLK_DIV16				1
#define GMACB_CLK_DIV32				2
#define GMACB_CLK_DIV48				3
#define GMACB_CLK_DIV64				4

/* Constants for MAN register */
#define GMACB_MAN_SOF				1
#define GMACB_MAN_WRITE				1
#define GMACB_MAN_READ				2
#define GMACB_MAN_CODE				2

/* Bit manipulation macros */
#define MACB_BIT(name)					\
	(1 << GMACB_##name##_OFFSET)
#define MACB_BF(name,value)				\
	(((value) & ((1 << GMACB_##name##_SIZE) - 1))	\
	 << GMACB_##name##_OFFSET)
#define MACB_BFEXT(name,value)\
	(((value) >> GMACB_##name##_OFFSET)		\
	 & ((1 << GMACB_##name##_SIZE) - 1))
#define MACB_BFINS(name,value,old)			\
	(((old) & ~(((1 << GMACB_##name##_SIZE) - 1)	\
		    << GMACB_##name##_OFFSET))		\
	 | MACB_BF(name,value))

/* Register access macros */
#define macb_readl(port,reg)				\
	readl((port)->regs + GMACB_##reg)
#define macb_writel(port,reg,value)			\
	writel((value), (port)->regs + GMACB_##reg)

#endif /* __DRIVERS_MACB_H__ */
