#ifndef _OMAP3_EGF_CPLD_H_
#define _OMAP3_EGF_CPLD_H_

#define GPIO_DISPLAY_POWER_ON       (1<<0)
#define GPIO_AUDIO_BT_LINSEL		    (1<<1)
#define GPIO_AUDIO_BT_MIC_MUTE      (1<<2)
#define GPIO_AUDIO_BT_EAR_MUTE      (1<<3)
#define GPIO_BT_RESET   			      (1<<4)
#define GPIO_WIFI_RESET       		  (1<<5)
#define GPIO_TELIT_ON_OFF        		(1<<6)
#define GPIO_TELIT_RESET        		(1<<7)
#define GPIO_USB_MM1_SUSPEND	      (1<<8)
#define GPIO_USB_MM3_SUSPEND   	    (1<<9)
#define GPIO_LEDC                   (1<<10)
#define GPIO_AMPLI_AGC1             (1<<11)
#define GPIO_AMPLI_AGC2             (1<<12)
#define GPIO_AMPLI_EN               (1<<13)
#define GPIO_MUX1_NRESET_3v3        (1<<14)
#define GPIO_TVP5150_ENABLE_3V3     (1<<15)
#define GPIO_TVP5150_NRESET_3V3     (1<<16)
#define GPIO_5VCAMSAT   						(1<<17)
#define GPIO_3V3       							(1<<18)
#define GPIO_5V0USB     						(1<<19)
#define GPIO_MIC_NSHDN              (1<<20)
#define GPIO_MCSPI4_RSTN            (1<<21)
#define GPIO_MCSPI4_PME             (1<<22)
#define GPIO_MEMS_EN_1V8          				(1<<23)
#define GPIO_DISPLAY_NRESET         (1<<24)

#define CPLD_READ_GPIO_OUT_MASK			0x80000000
#define CPLD_WRITE_MUXING_MASK			0x40000000

void set_cpld_gpio(unsigned int gpio_mask, int val);
int  get_cpld_gpio(unsigned int gpio_mask);
void set_cpld_muxing (unsigned int muxing_val);
void init_cpld_gpio(void);


#endif
