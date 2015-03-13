#ifndef GF_EEPROM_PORT_H_
#define GF_EEPROM_PORT_H_

#include <common.h>

#define SOM_EEPROM_I2C_BUS_NO 1
#define SOM_EEPROM_I2C_ADDRESS 0x50
#define BOARD_EEPROM_I2C_BUS_NO 2
#define BOARD_EEPROM_I2C_ADDRESS 0x56

#define PRODUCT_CODE_LEN 8
#define SW_ID_LEN 15

#define WID_TABLE_LENGTH 8

#define DEBUG_LEVEL 1

#define gf_debug(dbg_level,fmt,args...) \
	if (dbg_level<=DEBUG_LEVEL) printf(fmt, ##args); \
	else (void)0

#define GF_ATTRIBUTES 

struct wid_translation {
	char product_code[PRODUCT_CODE_LEN + 1];
	char sw_id_code[SW_ID_LEN + 1];
};

extern struct wid_translation legacy_wid_translation_table[WID_TABLE_LENGTH];

void gf_i2c_init(void);
int gf_i2c_set_bus_num(unsigned int bus);
int gf_i2c_probe (u8 chip);
void gf_serial_init(void);
int gf_serial_getc(void);
void gf_som_eeprom_unlock(void);
void gf_som_eeprom_lock(void);
int gf_eeprom_read(u8 address,u16 start_address,u8 * buffer,int len);
int gf_eeprom_write(u8 address,u16 start_address,u8 * buffer,int len);
int gf_read_programmer_file(const char * file_name,char * file_buffer,int buffer_length);
#endif /* GF_EEPROM_PORT_H_ */
