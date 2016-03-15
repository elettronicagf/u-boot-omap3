#ifndef _OMAP3_EGF_CPLD_H_
#define _OMAP3_EGF_CPLD_H_

#define EEPROM_WP_107      		(1<<0)
#define EXT_DISPLAY_EN_3V3		(1<<1)
#define DISP_1V8_NEN_3V3	    (1<<2)
#define AUDIO_SEL1_1V8      	(1<<3)
#define AUDIO_SEL2_1V8			(1<<4)
#define AUDIO_SEL3_1V8      	(1<<5)
#define AUDIO_SEL4_1V8       	(1<<6)
#define AUDIO_SEL_MS_R_1V8      (1<<7)
#define AUDIO_SEL_MS_L_1V8  	(1<<8)
#define MMC3_WF_RESET_3V3   	(1<<9)
#define MMC3_BT_RESET_3V3       (1<<10)
#define MMC3_ENABLE_3V3         (1<<11)
#define CPLD_EXP_06       		(1<<12)
#define CPLD_EXP_07         	(1<<13)

#define CPLD_MUX_EXP_01_OUT			(1<<0)
#define CPLD_MUX_EXP_02_OUT			(1<<1)
#define CPLD_MUX_EXP_03_OUT			(1<<2)
#define CPLD_MUX_EXP_04_OUT			(1<<3)
#define CPLD_MUX_EXP_05_OUT			(1<<4)
#define CPLD_MUX_EXP_06_OUT			(1<<5)
#define CPLD_MUX_EXP_07_OUT			(1<<6)
#define CPLD_MUX_EXP_08_OUT			(1<<7)
#define CPLD_MCSPI1_CS1_ENABLED		(1<<8)
#define CPLD_MCSPI1_CS1_ACTIVE_HIGH	(1<<9)


#define CPLD_READ_GPIO_OUT_MASK			0x80000000
#define CPLD_WRITE_MUXING_MASK			0x40000000

void set_cpld_gpio(unsigned int gpio_mask, int val);
int  get_cpld_gpio(unsigned int gpio_mask);
void set_cpld_muxing (unsigned int muxing_val);
void init_cpld_gpio(void);


#endif
