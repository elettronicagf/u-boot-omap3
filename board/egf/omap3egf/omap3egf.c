/*
 * (C) Copyright 2011
 * Elettronica GF s.r.l., <www.elettronicagf.it>
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

#include <common.h>
#include <twl4030.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/omap_gpio.h>
#include <asm/mach-types.h>
#ifdef CONFIG_USB_EHCI
#include <usb.h>
#include <asm/arch/clocks.h>
#include <asm/arch/clocks_omap3.h>
#include <asm/arch/ehci_omap3.h>
#include <asm/arch-omap3/omap3_spi.h>
#include <asm/arch-omap3/omap3_egf_cpld.h>
/* from drivers/usb/host/ehci-core.h */
extern struct ehci_hccr *hccr;
extern volatile struct ehci_hcor *hcor;
#endif
#include "omap3egf.h"
#include "muxing/pinmux_mbugrf-1.h"
#include <i2c.h>
#include "revision_codes.h"

#define pr_debug(fmt, args...) debug(fmt, ##args)

#define TWL4030_I2C_BUS			0

/* EEPROM */
#define EEPROM_I2C_BUS 			1

#define OMAP3EGF_GPIO_ETH_RST		64

DECLARE_GLOBAL_DATA_PTR;

static __u32 egf_product_code;

static __u32 get_product_code(void)
{
//	__u8 product_version[PRODUCT_VERSION_LEN];
//	u32 product_code;
//	int i;
//	i2c_set_bus_num(EEPROM_I2C_BUS);
//	for(i=0; i<PRODUCT_VERSION_LEN-1; i++){
//		if(i2c_read_byte_16bitoffset(0x50, i, &product_version[i])){
//			printf("EEPROM16 read Error\n");
//		}
//	}
//	i2c_set_bus_num(TWL4030_I2C_BUS);
//	product_code = REV_STR_TO_REV_CODE(product_version);
//	if(product_code != REV_NOT_PROGRAMMED){
//		product_version[PRODUCT_VERSION_LEN-1]=0; /* add termination character */
//		printf("Product = %s RevisionCode = %x\n",product_version,product_code);
//	}
//	else {
//		printf("Eeprom not programmed. Selected Default Configuration.\n");
//	}
//	return product_code;
	return 0;
}

int load_revision(void)
{
	egf_product_code = get_product_code();
	return 0;
}

void init_board_gpios()
{
	/* Leave tvp5150 enable and reset pins in a consistent state */
//	omap_request_gpio(163);
//	omap_request_gpio(164);
//	omap_set_gpio_direction(163,0);
//	omap_set_gpio_direction(164,0);
//	omap_set_gpio_dataout(163,1);
//	omap_set_gpio_dataout(164,0);
	return;
}
/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	init_board_gpios();
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP3_EGF;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}


/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	struct control_prog_io *prog_io_base = (struct control_prog_io *)OMAP34XX_CTRL_BASE;


	#ifdef CONFIG_DRIVER_OMAP34XX_I2C
		printf("i2c init...\n");
		i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	#endif


	/* Enable i2c2 pullup resisters */
	writel(~(PRG_I2C2_PULLUPRESX), &prog_io_base->io1);

	twl4030_power_init();
	twl4030_led_init(TWL4030_LED_LEDEN_LEDAON | TWL4030_LED_LEDEN_LEDBON);

	omap3_spi_init();

	load_revision();
	printf("Init CPLD...\n");
	init_cpld_gpio();
	printf("Init CPLD Muxing");
	set_cpld_muxing(CPLD_MUX_EXP_01_OUT | CPLD_MUX_EXP_02_OUT | CPLD_MUX_EXP_03_OUT |
			CPLD_MUX_EXP_04_OUT | CPLD_MUX_EXP_05_OUT | CPLD_MUX_EXP_06_OUT |
			CPLD_MUX_EXP_07_OUT | CPLD_MUX_EXP_08_OUT);

	/* Power display on */
	set_cpld_gpio(LCD_VDD_EN,1);

#if defined(CONFIG_CMD_NET)
	setup_net_chip();
	reset_net_chip();
#endif


	dieid_num_r();

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_MBUGRF_1();
}

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0);
	return 0;
}
#endif

#ifdef CONFIG_CMD_NET
/*
 * Routine: setup_net_chip
 * Description: Setting up the configuration GPMC registers specific to the
 *		Ethernet hardware.
 */
static void setup_net_chip(void)
{
	struct ctrl *ctrl_base = (struct ctrl *)OMAP34XX_CTRL_BASE;

	/* Configure GPMC registers */
	writel(NET_GPMC_CONFIG1, &gpmc_cfg->cs[5].config1);
	writel(NET_GPMC_CONFIG2, &gpmc_cfg->cs[5].config2);
	writel(NET_GPMC_CONFIG3, &gpmc_cfg->cs[5].config3);
	writel(NET_GPMC_CONFIG4, &gpmc_cfg->cs[5].config4);
	writel(NET_GPMC_CONFIG5, &gpmc_cfg->cs[5].config5);
	writel(NET_GPMC_CONFIG6, &gpmc_cfg->cs[5].config6);
	writel(NET_GPMC_CONFIG7, &gpmc_cfg->cs[5].config7);

	/* Enable off mode for NWE in PADCONF_GPMC_NWE register */
	writew(readw(&ctrl_base ->gpmc_nwe) | 0x0E00, &ctrl_base->gpmc_nwe);
	/* Enable off mode for NOE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_noe) | 0x0E00, &ctrl_base->gpmc_noe);
	/* Enable off mode for ALE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_nadv_ale) | 0x0E00,
		&ctrl_base->gpmc_nadv_ale);
}

/**
 * Reset the ethernet chip.
 */
static void reset_net_chip(void)
{
	int ret;
	int rst_gpio;

	rst_gpio = OMAP3EGF_GPIO_ETH_RST;

	ret = omap_request_gpio(rst_gpio);
	if (ret < 0) {
		printf("Unable to get GPIO %d\n", rst_gpio);
		return ;
	}

	/* Configure as output */
	omap_set_gpio_direction(rst_gpio, 0);

	/* Send a pulse on the GPIO pin */
	omap_set_gpio_dataout(rst_gpio, 1);
	udelay(1);
	omap_set_gpio_dataout(rst_gpio, 0);
	udelay(1);
	omap_set_gpio_dataout(rst_gpio, 1);
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}
#endif /* CONFIG_CMD_NET */


#ifdef CONFIG_USB_EHCI

#define GPIO_PHY_RESET 16

/* Reset is needed otherwise the kernel-driver will throw an error. */
int ehci_hcd_stop(void)
{
	pr_debug("Resetting OMAP3 EHCI\n");
	omap_set_gpio_dataout(GPIO_PHY_RESET, 0);
	writel(OMAP_UHH_SYSCONFIG_SOFTRESET, OMAP3_UHH_BASE + OMAP_UHH_SYSCONFIG);
	return 0;
}

/* Call usb_stop() before starting the kernel */
void show_boot_progress(int val)
{
	if(val == 15)
		usb_stop();
}

/*
 * Initialize the OMAP3 EHCI controller and PHY on the BeagleBoard.
 * Based on "drivers/usb/host/ehci-omap.c" from Linux 2.6.37.
 * See there for additional Copyrights.
 */
int ehci_hcd_init(void)
{
	pr_debug("Initializing OMAP3 ECHI\n");

	/* Put the PHY in RESET */
	omap_request_gpio(GPIO_PHY_RESET);
	omap_set_gpio_direction(GPIO_PHY_RESET, 0);
	omap_set_gpio_dataout(GPIO_PHY_RESET, 0);

	/* Hold the PHY in RESET for enough time till DIR is high */
	/* Refer: ISSUE1 */
	udelay(10);

	struct prcm *prcm_base = (struct prcm *)PRCM_BASE;
	/* Enable USBHOST_L3_ICLK (USBHOST_MICLK) */
	sr32(&prcm_base->iclken_usbhost, 0, 1, 1);
	/*
	 * Enable USBHOST_48M_FCLK (USBHOST_FCLK1)
	 * and USBHOST_120M_FCLK (USBHOST_FCLK2)
	 */
	sr32(&prcm_base->fclken_usbhost, 0, 2, 3);
	/* Enable USBTTL_ICLK */
	sr32(&prcm_base->iclken3_core, 2, 1, 1);
	/* Enable USBTTL_FCLK */
	sr32(&prcm_base->fclken3_core, 2, 1, 1);
	pr_debug("USB clocks enabled\n");

	/* perform TLL soft reset, and wait until reset is complete */
	writel(OMAP_USBTLL_SYSCONFIG_SOFTRESET,
		OMAP3_USBTLL_BASE + OMAP_USBTLL_SYSCONFIG);
	/* Wait for TLL reset to complete */
	while (!(readl(OMAP3_USBTLL_BASE + OMAP_USBTLL_SYSSTATUS)
			& OMAP_USBTLL_SYSSTATUS_RESETDONE));
	pr_debug("TLL reset done\n");

	writel(OMAP_USBTLL_SYSCONFIG_ENAWAKEUP |
		OMAP_USBTLL_SYSCONFIG_SIDLEMODE |
		OMAP_USBTLL_SYSCONFIG_CACTIVITY,
		OMAP3_USBTLL_BASE + OMAP_USBTLL_SYSCONFIG);

	/* Put UHH in NoIdle/NoStandby mode */
	writel(OMAP_UHH_SYSCONFIG_ENAWAKEUP
		| OMAP_UHH_SYSCONFIG_SIDLEMODE
		| OMAP_UHH_SYSCONFIG_CACTIVITY
		| OMAP_UHH_SYSCONFIG_MIDLEMODE,
		OMAP3_UHH_BASE + OMAP_UHH_SYSCONFIG);

	/* setup burst configurations */
	writel(OMAP_UHH_HOSTCONFIG_INCR4_BURST_EN
		| OMAP_UHH_HOSTCONFIG_INCR8_BURST_EN
		| OMAP_UHH_HOSTCONFIG_INCR16_BURST_EN,
		OMAP3_UHH_BASE + OMAP_UHH_HOSTCONFIG);

	/*
	 * Refer ISSUE1:
	 * Hold the PHY in RESET for enough time till
	 * PHY is settled and ready
	 */
	udelay(10);
	omap_set_gpio_dataout(GPIO_PHY_RESET, 1);

	hccr = (struct ehci_hccr *)(OMAP3_EHCI_BASE);
	hcor = (struct ehci_hcor *)(OMAP3_EHCI_BASE + 0x10);

	pr_debug("OMAP3 EHCI init done\n");
	return 0;
}

#endif /* CONFIG_USB_EHCI */
