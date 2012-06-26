#include <common.h>

/* physical address of device */

#define OMAP2_MCSPI_REVISION    (0x00 >> 2)
#define OMAP2_MCSPI_SYSCONFIG   (0x10 >> 2)
#define OMAP2_MCSPI_SYSSTATUS   (0x14 >> 2)
#define OMAP2_MCSPI_IRQSTATUS   (0x18 >> 2)
#define OMAP2_MCSPI_IRQENABLE   (0x1c >> 2)
#define OMAP2_MCSPI_WAKEUPENABLE  (0x20 >> 2)
#define OMAP2_MCSPI_SYST    		(0x24 >> 2)
#define OMAP2_MCSPI_MODULCTRL   (0x28 >> 2)
#define OMAP2_MCSPI_SYSCONF_DISABLE_IDLE (0x01 << 3)
#define OMAP2_MCSPI_SYSCONF_MANTAINS_CLOCKS (0x03 << 8)

/* per-channel banks, 0x14 bytes each, first is: */
#define OMAP2_MCSPI_CHCONF   (0x2c >> 2)
#define OMAP2_MCSPI_CHSTAT   (0x30 >> 2)
#define OMAP2_MCSPI_CHCTRL   (0x34 >> 2)
#define OMAP2_MCSPI_TX     	 (0x38 >> 2)
#define OMAP2_MCSPI_RX       (0x3c >> 2)

#define BIT(nr)                 (1UL << (nr))
#define OMAP2_MCSPI_SYSCONFIG_SMARTIDLE BIT(4)
#define OMAP2_MCSPI_SYSCONFIG_ENAWAKEUP BIT(2)
#define OMAP2_MCSPI_SYSCONFIG_AUTOIDLE  BIT(0)
#define OMAP2_MCSPI_SYSCONFIG_SOFTRESET BIT(1)
#define OMAP2_MCSPI_SYSSTATUS_RESETDONE BIT(0)
#define OMAP2_MCSPI_MODULCTRL_SINGLE  BIT(0)
#define OMAP2_MCSPI_MODULCTRL_MS  BIT(2)
#define OMAP2_MCSPI_MODULCTRL_STEST BIT(3)

#define OMAP2_MCSPI_CHCONF_PHA    BIT(0)
#define OMAP2_MCSPI_CHCONF_POL    BIT(1)
#define OMAP2_MCSPI_CHCONF_CLKD_MASK  (0x0f << 2)
#define OMAP2_MCSPI_CHCONF_EPOL   BIT(6)
#define OMAP2_MCSPI_CHCONF_WL_MASK  (0x1f << 7)
#define OMAP2_MCSPI_CHCONF_TRM_RX_ONLY  BIT(12)
#define OMAP2_MCSPI_CHCONF_TRM_TX_ONLY  BIT(13)
#define OMAP2_MCSPI_CHCONF_TRM_MASK (0x03 << 12)

#define OMAP2_MCSPI_CHCONF_DMAW   BIT(14)
#define OMAP2_MCSPI_CHCONF_DMAR   BIT(15)
#define OMAP2_MCSPI_CHCONF_DPE0   BIT(16)
#define OMAP2_MCSPI_CHCONF_DPE1   BIT(17)
#define OMAP2_MCSPI_CHCONF_IS   BIT(18)
#define OMAP2_MCSPI_CHCONF_TURBO  BIT(19)
#define OMAP2_MCSPI_CHCONF_FORCE  BIT(20)

#define OMAP2_MCSPI_CHSTAT_RXS    BIT(0)
#define OMAP2_MCSPI_CHSTAT_TXS    BIT(1)
#define OMAP2_MCSPI_CHSTAT_EOT    BIT(2)

#define OMAP2_MCSPI_CHCTRL_EN   BIT(0)

#define OMAP2_MCSPI_WAKEUPENABLE_WKEN BIT(0)

#define OMAP2_CM_FCLKEN1_CORE   0xA00 >> 2
#define OMAP2_CM_ICLKEN1_CORE   0xA10 >> 2

#define OMAP2_MCSPI4_CLK_ENABLE BIT(21)
#define OMAP2_MCSPI3_CLK_ENABLE BIT(20)
#define OMAP2_MCSPI2_CLK_ENABLE BIT(19)
#define OMAP2_MCSPI1_CLK_ENABLE BIT(18)

#define OMAP2_MCSPI1_BASE     0x48098000
#define OMAP2_MCSPI2_BASE     0x4809A000
#define OMAP2_MCSPI3_BASE     0x480B8000
#define OMAP2_MCSPI4_BASE     0x480BA000

#define OMAP2_MCSPI_IRQSTAT_RX0_FULL    BIT(2)
#define OMAP2_MCSPI_IRQSTAT_TX0_EMPTY   BIT(0)
//#define VERBOSE

static volatile unsigned int* pcrm_base = (unsigned int*) 0x48004000;

static void enableClocks(int nController) {
	pcrm_base[OMAP2_CM_ICLKEN1_CORE] |= (OMAP2_MCSPI1_CLK_ENABLE
			<< (nController - 1));
	pcrm_base[OMAP2_CM_FCLKEN1_CORE] |= (OMAP2_MCSPI1_CLK_ENABLE
			<< (nController - 1));
}

static void disableClocks(int nController) {
	pcrm_base[OMAP2_CM_ICLKEN1_CORE] &= ~(OMAP2_MCSPI1_CLK_ENABLE
			<< (nController - 1));
	pcrm_base[OMAP2_CM_FCLKEN1_CORE] &= ~(OMAP2_MCSPI1_CLK_ENABLE
			<< (nController - 1));
}

static void enableCtrl(volatile unsigned int* base, int nChannel) {
	volatile unsigned int* ctrl_reg;
	ctrl_reg = base + OMAP2_MCSPI_CHCTRL + 5 * nChannel;
	(*ctrl_reg) |= OMAP2_MCSPI_CHCTRL_EN;
}

static void disableCtrl(volatile unsigned int* base, int nChannel) {
	volatile unsigned int* ctrl_reg;
	ctrl_reg = base + OMAP2_MCSPI_CHCTRL + 5 * nChannel;
	(*ctrl_reg) &= (~OMAP2_MCSPI_CHCTRL_EN);
}

static void enableCS(volatile unsigned int* base, int nChannel) {
	volatile unsigned int* conf_reg;
	conf_reg = base + OMAP2_MCSPI_CHCONF + 5 * nChannel;
	(*conf_reg) |= OMAP2_MCSPI_CHCONF_FORCE;
}

static void setTransmitOnly(volatile unsigned int* base, int nChannel) {
	volatile unsigned int* conf_reg;
	conf_reg = base + OMAP2_MCSPI_CHCONF + 5 * nChannel;
	(*conf_reg) &= ~(OMAP2_MCSPI_CHCONF_TRM_MASK);
	(*conf_reg) |= OMAP2_MCSPI_CHCONF_TRM_TX_ONLY;
}

static void setTransmitReceive(volatile unsigned int* base, int nChannel) {
	volatile unsigned int* conf_reg;
	conf_reg = base + OMAP2_MCSPI_CHCONF + 5 * nChannel;
	(*conf_reg) &= ~(OMAP2_MCSPI_CHCONF_TRM_MASK);
}

static void disableCS(volatile unsigned int* base, int nChannel) {
	volatile unsigned int* conf_reg;
	conf_reg = base + OMAP2_MCSPI_CHCONF + 5 * nChannel;

	(*conf_reg) &= ~OMAP2_MCSPI_CHCONF_FORCE;
}

static void writeTx(volatile unsigned int* base, int nChannel, unsigned int val) {
	volatile unsigned int tmp;
	volatile unsigned int* tx_reg;

	int cnt = 0;

#ifdef VERBOSE
	printf("\nsending %x\n" , val );
#endif
	base[OMAP2_MCSPI_IRQSTATUS] &= (OMAP2_MCSPI_IRQSTAT_TX0_EMPTY
			<< (4 * nChannel)); //reset event
	base[OMAP2_MCSPI_IRQSTATUS] &= (OMAP2_MCSPI_IRQSTAT_RX0_FULL
			<< (4 * nChannel)); //reset event

	tmp = base[OMAP2_MCSPI_IRQSTATUS];
#ifdef VERBOSE
	printf("before writing TX register OMAP2_MCSPI_IRQSTATUS =%x\n",tmp);
#endif
	tx_reg = base + OMAP2_MCSPI_TX + 5 * nChannel;
	*tx_reg = val;
	tmp = base[OMAP2_MCSPI_IRQSTATUS];
#ifdef VERBOSE
	printf("before waiting empty TX register OMAP2_MCSPI_IRQSTATUS =%x\n",tmp);
#endif

	while (!(tmp & (OMAP2_MCSPI_IRQSTAT_TX0_EMPTY << (4 * nChannel)))) {
		tmp = base[OMAP2_MCSPI_IRQSTATUS];
		cnt++;
		if (cnt > 10000) {
			printf("SPI Reading Error, TX not empty.\n");
			break;
		}
	}
#ifdef VERBOSE
	printf("after empty TX register OMAP2_MCSPI_IRQSTATUS =%x\n",tmp);
#endif

	return;
}

unsigned int readRx(volatile unsigned int* base, int nChannel) {
	int cnt = 0;
	volatile unsigned int tmp;
	unsigned int v = 0;
	volatile unsigned int* rx_reg;
	tmp = base[OMAP2_MCSPI_IRQSTATUS];
#ifdef VERBOSE
	printf("before waiting empty RX full OMAP2_MCSPI_IRQSTATUS =%x\n",tmp);
#endif
	while (!(tmp & (OMAP2_MCSPI_IRQSTAT_RX0_FULL << (4 * nChannel)))) {
		tmp = base[OMAP2_MCSPI_IRQSTATUS];
		//nanosleep(&ts,&tsrem);
		cnt++;
		if (cnt > 10000) {
			printf("SPI Reading Error, RX not full.\n");
			break;
		}

	}
#ifdef VERBOSE
	printf("after empty RX full OMAP2_MCSPI_IRQSTATUS =%x\n",tmp);
#endif

	rx_reg = base + OMAP2_MCSPI_RX + 5 * nChannel;
	v = *rx_reg;
#ifdef VERBOSE
	printf("Ricevuto %x\n", v);
#endif

	return v;

}

static void resetController(volatile unsigned int* base) {
	volatile unsigned int tmp;
	base[OMAP2_MCSPI_SYSCONFIG] |= 0x2;
	tmp = base[OMAP2_MCSPI_SYSSTATUS];
#ifdef VERBOSE
	printf("waiting for reset OMAP2_MCSPI_SYSSTATUS %x\n", tmp);
#endif
	while (!(tmp & 0x01)) {
		tmp = base[OMAP2_MCSPI_SYSSTATUS];
#ifdef VERBOSE
		printf("waiting for reset OMAP2_MCSPI_SYSSTATUS %x\n", tmp);
#endif
	}
#ifdef VERBOSE
	printf("Reset OK\n");
#endif

}
static volatile unsigned int* get_spi_base(int nController) {
	switch (nController) {
	case 1:
		return (unsigned int *) OMAP2_MCSPI1_BASE;
	case 2:
		return (unsigned int *) OMAP2_MCSPI2_BASE;
	case 3:
		return (unsigned int *) OMAP2_MCSPI3_BASE;
	case 4:
		return (unsigned int *) OMAP2_MCSPI4_BASE;
	default:
		printf("Error unknown SPI Controller\n");
		return 0;
	}
}
static void setup_spi_channel(volatile unsigned int* base, int nChannel,
		unsigned int configuration) {
	volatile unsigned int* conf_reg;
	conf_reg = base + OMAP2_MCSPI_CHCONF + 5 * nChannel;
	*conf_reg = configuration;
}

static void setup_spi_mastermode_forced_cs(volatile unsigned int* base) {

	base[OMAP2_MCSPI_SYSCONFIG] = 0x15;
	base[OMAP2_MCSPI_MODULCTRL] |= OMAP2_MCSPI_MODULCTRL_SINGLE;
	base[OMAP2_MCSPI_MODULCTRL] &= ~OMAP2_MCSPI_MODULCTRL_MS;
	base[OMAP2_MCSPI_WAKEUPENABLE] |= OMAP2_MCSPI_WAKEUPENABLE_WKEN;
}

void omap3_spi_send_receive(int nController, int nChannel, unsigned int val,
		unsigned int* response) {
	volatile unsigned int* base = get_spi_base(nController);
	enableClocks(nController);
	setTransmitReceive(base, nChannel);
	enableCtrl(base, nChannel);
	enableCS(base, nChannel);
	writeTx(base, nChannel, val);
	(*response) = readRx(base, nChannel);
	disableCS(base, nChannel);
	disableCtrl(base, nChannel);
	disableClocks(nController);
}

void omap3_spi_send(int nController, int nChannel, unsigned int val) {
	volatile unsigned int* base = get_spi_base(nController);
	enableClocks(nController);
	setTransmitOnly(base, nChannel);
	enableCtrl(base, nChannel);
	enableCS(base, nChannel);
	writeTx(base, nChannel, val);
	disableCS(base, nChannel);
	disableCtrl(base, nChannel);
	disableClocks(nController);
}

void omap3_spi_send_array(int nController, int nChannel, unsigned int* val,
		unsigned int nMsg) {
	volatile unsigned int* base = get_spi_base(nController);
	int i;
	enableClocks(nController);
	setTransmitOnly(base, nChannel);
	enableCtrl(base, nChannel);
	enableCS(base, nChannel);
	for (i = 0; i < nMsg; i++) {
		writeTx(base, nChannel, val[i]);
		udelay(10);
	}
	disableCS(base, nChannel);
	disableCtrl(base, nChannel);
	disableClocks(nController);
}

void omap3_spi_init(void) {
	volatile unsigned int* spi_base;
	int nController;

	//Init Controller 1
	nController = 1;
	enableClocks(nController);
	spi_base = get_spi_base(nController);
	resetController(spi_base);
	setup_spi_mastermode_forced_cs(spi_base);
	//Forced initialization of every channel to avoid ChipSelect problems
	setup_spi_channel(spi_base, 0, 0x0010Fe3); //Channel 0 CPLD SPI MODE_3 300Khz 32 bit TX/RX
	setup_spi_channel(spi_base, 1, 0x00107e7); // Amoled Init
	setup_spi_channel(spi_base, 2, 0x00103d8); // Touch screen

}
