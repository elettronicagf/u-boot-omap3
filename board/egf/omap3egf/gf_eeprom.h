/*
 * gf_eeprom.h
 *
 *  Library for eGF eeprom management
 *  Author: Stefano Donati
 *  Date: 28/11/2013
 *  Version: 1.0.0
 */

#ifndef GF_EEPROM_H_
#define GF_EEPROM_H_

#define GF_EEPROM_SW_VERSION "1.0.0"

#define PRODUCT_VERSION_LEN  12  /* termination character included. ex: JSC0336_A02*/

#define EEPROM_MAX_N_BLOCKS 2
#define EEPROM_BLOCK_LEN 128

#define EEPROM_FOUND	0
#define EEPROM_NOT_FOUND 	1
#define EEPROM_UNKNOWN_PROTOCOL -1

#define EEPROM_LATEST_PROTOCOL_VERSION 3
#define MAC_ADDRESS_STR_LEN 17
#define WID_STR_LEN 15
#define PC_STR_LEN 30

struct gf_config {
	/* Software identification ID */
	char wid[WID_STR_LEN + 1];
	/* Product code string */
	char product_code[PC_STR_LEN + 1];
};

char * gf_eeprom_get_som_code(void);
char * gf_eeprom_get_board_code(void);
char * gf_eeprom_get_mac1_address(void);
char * gf_eeprom_get_mac2_address(void);
char * gf_eeprom_get_som_serial_number(void);
char * gf_eeprom_get_board_serial_number(void);
char * gf_eeprom_get_som_sw_id_code(void);
char * gf_eeprom_get_board_sw_id_code(void);

int check_eeprom_hw_som(void);
int check_eeprom_hw_board(void);
int identify_eeprom_protocol_som(void);
int identify_eeprom_protocol_board(void);

void program_som_eeprom(struct gf_config *config);
void program_board_eeprom(struct gf_config *config);

void upgrade_som_eeprom_to_latest_version(void);
void upgrade_board_eeprom_to_latest_version(void);

void init_gf_som_eeprom(void);
void init_gf_board_eeprom(void);
void load_som_eeprom(void);
void load_board_eeprom(void);

int gf_load_som_revision(char ** egf_sw_id_code, int bypass_checks);

#endif /* GF_EEPROM_H_ */
