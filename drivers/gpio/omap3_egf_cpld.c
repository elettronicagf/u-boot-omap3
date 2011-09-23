#include <common.h>
#include <asm/arch-omap3/omap3_egf_cpld.h>
#include <asm/arch-omap3/omap3_spi.h>

static unsigned int cpld_out_reg;

void set_cpld_gpio(unsigned int gpio_mask, int val)
{
	unsigned int response;
	if(val){
		cpld_out_reg |= gpio_mask;
	}
	else{
		cpld_out_reg &= ~gpio_mask;
	}
	
	omap3_spi_send_receive(CPLD_GPIO_SPI_CONTROLLER, CPLD_GPIO_SPI_CS, cpld_out_reg, &response);
	
}

void set_cpld_muxing (unsigned int muxing_val){
	unsigned int response;
	unsigned int message=muxing_val;
	message |= CPLD_WRITE_MUXING_MASK;
	printf("CPLD: Inviato %d\n",message);
	omap3_spi_send_receive(CPLD_GPIO_SPI_CONTROLLER,CPLD_GPIO_SPI_CS,message,&response);
	printf("CPLD: Ricevuto %d\n",response);
}

static void print_cpld_version(void)
{
	unsigned int response;
	omap3_spi_send_receive(CPLD_GPIO_SPI_CONTROLLER, CPLD_GPIO_SPI_CS, cpld_out_reg, &response);
	int major = (response >> 22) & 0x0F;
	int minor = (response >> 14) & 0xFF;
	printf("Explor: CPLD Firmware Version %d.%d\n",major,minor);
}


void init_cpld_gpio(void){
/* Load cpld_out */
	omap3_spi_send_receive(CPLD_GPIO_SPI_CONTROLLER, CPLD_GPIO_SPI_CS, CPLD_READ_GPIO_OUT_MASK, &cpld_out_reg);
	print_cpld_version();
}
