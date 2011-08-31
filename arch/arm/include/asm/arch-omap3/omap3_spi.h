#ifndef _OMAP3_SPI_H_
#define _OMAP3_SPI_H_

void omap3_spi_send_receive(int nController, int nChannel, unsigned int val, unsigned int* response);
void omap3_spi_send_array(int nController, int nChannel, unsigned int* val, unsigned int nMsg);

void omap3_spi_send(int nController, int nChannel, unsigned int val);
void omap3_spi_init(void);

#define CPLD_GPIO_SPI_CONTROLLER		1
#define CPLD_GPIO_SPI_CS						0

#define DISPLAY_SPI_CONTROLLER			1
#define DISPLAY_SPI_CS							1


#endif

