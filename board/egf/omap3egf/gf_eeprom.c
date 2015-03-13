/*
 * gf_eeprom.c
 *
 *  Library for eGF eeprom management
 *  Author: Stefano Donati
 *  Date: 28/11/2013
 *  Version: 1.0.0
 */

#include "gf_eeprom.h"
#include "gf_eeprom_port.h"

#define EEPROM_SIZE 65536/8

#define MAX_FIELDS_NO 50
#define EEPROM_ID_CODE_START_ADDRESS 0
#define EEPROM_ID_CODE_LEN 2
#define EEPROM_SPACE_USED (EEPROM_MAX_N_BLOCKS*EEPROM_BLOCK_LEN)
#define EEPROM_ID_CODE "GF"
#define EEPROM_ID_CODE_LEGACY	"JS"

#define EEPROM_NOT_PROGRAMMED -1
#define EEPROM_PROGRAMMED	0

#define EEPROM_PROTOCOL_VERSION_START_ADDRESS EEPROM_ID_CODE_START_ADDRESS+EEPROM_ID_CODE_LEN
#define EEPROM_PROTOCOL_VERSION_LEN	 3

#define VERSION_STR_TO_VERSION_CODE(VERSION_STRING) \
	((VERSION_STRING[0]-'0')*100 + (VERSION_STRING[1]-'0')*10+(VERSION_STRING[2]-'0')*1)

#define ISSPACE(c)  ((c) == ' ' || ((c) >= '\t' && (c) <= '\r'))
#define ISASCII(c)  (((c) & ~0x7f) == 0)
#define ISUPPER(c)  ((c) >= 'A' && (c) <= 'Z')
#define ISLOWER(c)  ((c) >= 'a' && (c) <= 'z')
#define ISALPHA(c)  (ISUPPER(c) || ISLOWER(c))
#define ISDIGIT(c)  ((c) >= '0' && (c) <= '9')

#define EEPROM_PROGRAMMER_FILE_NAME "gfconfig"
#define EEPROM_PROGRAMMER_FILE_BUFFER_SIZE 100

struct eeprom_protocol {
	/* Verification ID. Include termination character */
	char eeprom_id[EEPROM_ID_CODE_LEN + 1];
	int version;
	/* total number of the field in the eeprom */
	int total_fields;
};

/*
 * EEPROM PROTOCOL GF Version 1
 * 0:ProductCode
 * 1:SerialNumber
 * 2:ProdDate
 * 3:TesterCode
 * 4:MACAddress
 * 5:Checksum
 */
struct eeprom_protocol GF_ATTRIBUTES eeprom_protocol_GF001 =
{ 	EEPROM_ID_CODE,
	1, /* Version */
	6 /* Number of Fields */
};

/*
 * EEPROM PROTOCOL GF Version 2
 * 0:ProductCode
 * 1:SerialNumber
 * 2:ProdDate
 * 3:TesterCode
 * 4:Checksum
 */
struct eeprom_protocol GF_ATTRIBUTES eeprom_protocol_GF002 =
{ 	EEPROM_ID_CODE,
	2, /* Version */
	5 /* Number of Fields */
};

/*
 * EEPROM PROTOCOL GF Version 3
 * 0:SW ID Code (Example WID0336_????.??)
 * 1:ProductCode with complete HW Conf (Example JSC0336_H01.beta1_Conf-001_A01)
 * 2:SerialNumber
 * 3:ProdDate
 * 4:TesterCode
 * 5:MACAddress
 * 6:Hex Checksum
 */
struct eeprom_protocol GF_ATTRIBUTES eeprom_protocol_GF003 =
{ 	EEPROM_ID_CODE,
	3, /* Version */
	7 /* Number of Fields */
};

struct eeprom {
	u8 bus_num;
	u8 i2c_address;
	struct eeprom_protocol protocol;
	/* buffer containing a copy of the first EEPROM_SPACE_USED bytes of eeprom data */
	char content[EEPROM_SPACE_USED];
	int status;
	int len;
	int address_len;
	/* List of pointer to the fields.
	 * Pointer point to eeprom_content starting char of each field.
	 * */
	char * fields[MAX_FIELDS_NO];
};

static struct eeprom GF_ATTRIBUTES som_eeprom;
static struct eeprom GF_ATTRIBUTES board_eeprom;

static  char GF_ATTRIBUTES gf_file_buffer[EEPROM_PROGRAMMER_FILE_BUFFER_SIZE];

struct gf_config read_config;

static void *gf_memset(void *s, int c, size_t n)
{
	size_t i;
	char *ptr = s;
	for (i=0;i<n;i++,ptr++)
	{
		*ptr = c;
	}
	return s;
}

static size_t gf_strlen(const char * s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc);
	return sc - s;
}

int gf_strncmp(const char * cs, const char * ct, size_t count) {
	register signed char __res = 0;

	while (count) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
		count--;
	}

	return __res;
}


void gf_strcpy (char dest[], const char src[])
{
	int i = 0;
	while (1)
	{
		dest[i] = src[i];
		if (dest[i] == '\0') break;
		i++;

	}
}

char * gf_strcat (char *dest, const char *src)
{
	while(*dest) dest++;
	while ((*dest++ = *src++));
	return dest;
}


unsigned long gfstr_toul (
    char*   nstr,
    char**  endptr,
    int base)
{
    char* s = nstr;
    unsigned long acc;
    unsigned char c;
    unsigned long cutoff;
    int neg = 0, any, cutlim;

    do
    {
        c = *s++;
    } while (ISSPACE(c));

    if (c == '-')
    {
        neg = 1;
        c = *s++;
    }
    else if (c == '+')
        c = *s++;

    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X'))
    {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    cutoff = -1UL/ (unsigned long)base;
    cutlim = -1UL % (unsigned long)base;
    for (acc = 0, any = 0; ; c = *s++)
    {
        if (!ISASCII(c))
            break;
        if (ISDIGIT(c))
            c -= '0';
        else if (ISALPHA(c))
            c -= ISUPPER(c) ? 'A' - 10 : 'a' - 10;
        else
            break;

        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
            any = -1;
        else
        {
            any = 1;
            acc *= base;
            acc += c;
        }
    }

    if (any < 0)
    {
        acc = ~0U >> 1;
    }
    else if (neg)
        acc = -acc;
    if (endptr != 0)
        *((const char **)endptr) = any ? s - 1 : nstr;
    return (acc);

}

static void gf_u8tox(u8 i, char *s)
{
    unsigned char n;

    s += 2;
    *s = '\0';

    for (n = 2; n != 0; --n) {
        *--s = "0123456789ABCDEF"[i & 0x0F];
        i >>= 4;
    }
}

static unsigned int checksum(struct eeprom* eep)
{
	unsigned int sum = 0;
	int i;
	char* s;

	for (i = 0; i < eep->protocol.total_fields - 1; i++) {
		s = eep->fields[i];
		while (*s != 0) {
			sum += *(unsigned char*) s;
			s++;
		}
	}
	return sum & 0XFF;

}

static char * get_sw_id_code(struct eeprom* eep) {
	if (eep->status == EEPROM_NOT_PROGRAMMED)
		return NULL;
	switch (eep->protocol.version) {
	case 3:
		return eep->fields[0];
		break;
	default:
		return NULL;
	}
}


static char * get_product_code(struct eeprom* eep) {
	if (eep->status == EEPROM_NOT_PROGRAMMED)
		return NULL;
	switch (eep->protocol.version) {
	case 1:
	case 2:
		return eep->fields[0];
		break;
	case 3:
		return eep->fields[1];
		break;
	default:
		return NULL;
	}
}

static  char * get_mac_address(struct eeprom* eep) {
	if (eep->status == EEPROM_NOT_PROGRAMMED)
		return NULL;
	switch (eep->protocol.version) {
	case 1:
		if (gf_strlen(eep->fields[4]) != MAC_ADDRESS_STR_LEN) {
			return NULL;
		}
		return eep->fields[4];
		break;
	case 3:
		if (gf_strlen(eep->fields[5]) != MAC_ADDRESS_STR_LEN) {
			return NULL;
		}
		return eep->fields[5];
		break;
	default:
		return NULL;
	}
}

static  char * get_serial_number(struct eeprom* eep) {
	if (eep->status == EEPROM_NOT_PROGRAMMED)
		return NULL;
	switch (eep->protocol.version) {
	case 1:
	case 2:
		return eep->fields[1];
		break;
	case 3:
		return eep->fields[2];
		break;
	default:
		return NULL;
	}
}



char * gf_eeprom_get_som_code(void)
{
	return get_product_code(&som_eeprom);
}
char * gf_eeprom_get_board_code(void)
{
	return get_product_code(&board_eeprom);
}
char * gf_eeprom_get_mac1_address(void){
	return get_mac_address(&som_eeprom);
}
char * gf_eeprom_get_mac2_address(void){
	return get_mac_address(&board_eeprom);
}
char * gf_eeprom_get_som_serial_number(void)
{
	return get_serial_number(&som_eeprom);
}
char * gf_eeprom_get_board_serial_number(void)
{
	return get_serial_number(&board_eeprom);
}
char * gf_eeprom_get_som_sw_id_code(void)
{
	return get_sw_id_code(&som_eeprom);
}
char * gf_eeprom_get_board_sw_id_code(void)
{
	return get_sw_id_code(&board_eeprom);
}

static void init_eeprom(struct eeprom* e, u8 busnum, u8 i2caddress, u8 addresslen) {
	e->bus_num = busnum;
	e->i2c_address = i2caddress;
	e->protocol.eeprom_id[0] = 0;
	e->protocol.version = -1;
	e->status = EEPROM_NOT_PROGRAMMED;
	e->len = 0;
	e->address_len = addresslen;
}

/* Probe eeprom to check HW functionality */
static int check_eeprom_hw(struct eeprom* eep){
	gf_i2c_set_bus_num(eep->bus_num);
	if (!gf_i2c_probe(eep->i2c_address)) {
		return EEPROM_FOUND;
	} else {
		return EEPROM_NOT_FOUND;
	}
}


int check_eeprom_hw_som(void){
	return check_eeprom_hw(&som_eeprom);
}

int check_eeprom_hw_board(void){
	return check_eeprom_hw(&board_eeprom);
}

static int eeprom_is_new(struct eeprom* eep){
	int i;
	char content[EEPROM_SPACE_USED];
	gf_memset(content,0xFF,EEPROM_SPACE_USED);
	gf_eeprom_read(eep->i2c_address, 0,
					(u8 *)content,EEPROM_SPACE_USED);
	for (i = 0; i < EEPROM_SPACE_USED; i++)
	{
		if(content[i]!=0xFF)
			return 0;
	}
	return 1;
}

int som_eeprom_is_new(void)
{
	return eeprom_is_new(&som_eeprom);
}

int board_eeprom_is_new(void)
{
	return eeprom_is_new(&board_eeprom);
}

/* Identify eeprom protocol version */
static int identify_eeprom_protocol(struct eeprom* eep){
	unsigned char tmpbuf[10];

	gf_i2c_set_bus_num(eep->bus_num);
	/* Read eeprom ID code */
	if (gf_eeprom_read(eep->i2c_address, EEPROM_ID_CODE_START_ADDRESS,
			(u8 *)eep->protocol.eeprom_id, EEPROM_ID_CODE_LEN)) {
		gf_debug(0,"Eeprom read error!\n");
		eep->status = EEPROM_NOT_PROGRAMMED;
		return EEPROM_UNKNOWN_PROTOCOL;
	}
	/* Terminate EEPROM ID string */
	eep->protocol.eeprom_id[EEPROM_ID_CODE_LEN] = 0;
	/* Check if read string is a valid ID */
	if (gf_strncmp(eep->protocol.eeprom_id, EEPROM_ID_CODE, eep->address_len)) {
		/* Check if eeprom is in protocol version 0.
		 * In that case, the first two characters are from product code string "JS"
		 */
		if (gf_strncmp(eep->protocol.eeprom_id, EEPROM_ID_CODE_LEGACY, EEPROM_ID_CODE_LEN)) {
			/* EEPROM ID string not found or not valid */
			gf_debug(0,"Eeprom verification ID not found\n");
			eep->status = EEPROM_NOT_PROGRAMMED;
			return EEPROM_UNKNOWN_PROTOCOL;
		} else {
			/* EEPROM ID string for protocol version 0 found */
			gf_debug(1,"Eeprom Protocol Version 0\n");
			eep->protocol.version = 0;
			return 0;
		}
	} else {
		/* EEPROM ID string for protocol version > 0 found */
		gf_debug(2,"Eeprom verification ID found: %s\n",eep->protocol.eeprom_id);
		/* Read EEPROM protocol version */
		if (gf_eeprom_read(eep->i2c_address, EEPROM_PROTOCOL_VERSION_START_ADDRESS,
				tmpbuf, EEPROM_PROTOCOL_VERSION_LEN)) {
			gf_debug(0,"Eeprom read Error\n");
			eep->status = EEPROM_NOT_PROGRAMMED;
			return EEPROM_UNKNOWN_PROTOCOL;
		}
		tmpbuf[EEPROM_PROTOCOL_VERSION_LEN] = 0;
		eep->protocol.version = VERSION_STR_TO_VERSION_CODE(tmpbuf);
		gf_debug(1,"Eeprom Protocol Version: %d\n", eep->protocol.version);
		return eep->protocol.version;
	}
}

int identify_eeprom_protocol_som(void){
	return identify_eeprom_protocol(&som_eeprom);
}

int identify_eeprom_protocol_board(void){
	return identify_eeprom_protocol(&board_eeprom);
}

static void gf_program_eeprom(struct eeprom* eep,struct gf_config *config){
	char content[EEPROM_SPACE_USED];
	int i = 0;
	char * s;
	u8 cs = 0;
	char cs_string[3];

	gf_debug(0,"Programming Eeprom to GF003\n");
	gf_memset(content,0xFF,sizeof(content));
	content[0]=0;
	/* Insert ID and Rev field for target eeprom content */
	gf_strcat(content,"GF003\n");
	/* Add SW ID field from configuration info to content */
	gf_strcat(content,config->wid);
	gf_strcat(content,"\n");
	/* Add product code field from configuration info to content */
	gf_strcat(content,config->product_code);
	gf_strcat(content,"\n");
	/* Add the remaining empty fields with the exception of checksum field */
	for (i = 0; i<eeprom_protocol_GF003.total_fields-3; i++)
	{
		gf_strcat(content,"\n");
	}

	/* Start computing checksum after GF003 protocol header */
	s = content+EEPROM_ID_CODE_LEN+EEPROM_PROTOCOL_VERSION_LEN+1;
	/* Calculate checksum */
	for (i = 0; i<eeprom_protocol_GF003.total_fields-1;i++)
	{
		while (*s != '\n') {
			cs = cs + *(unsigned char*) s;
			s++;
		}
		s++;
	}
	cs = cs & 0XFF;
	/* Convert checksum to string */
	gf_u8tox(cs,cs_string);
	/* Append checksum to eeprom content */
	gf_strcat(content,cs_string);
	gf_strcat(content,"\n");
	gf_debug(0,"Programming Eeprom...");
	/* Copy content to eeprom */
	gf_i2c_set_bus_num(eep->bus_num);
	if(gf_eeprom_write(eep->i2c_address,0,(unsigned char *) content,EEPROM_SPACE_USED))
	{
		gf_debug(0,"\nEeprom Program Failed. Write error,\n");
		return;
	}
	gf_debug(0,"Done\n");
	return;
}

void program_som_eeprom(struct gf_config *config){
	gf_program_eeprom(&som_eeprom,config);
}

void program_board_eeprom(struct gf_config *config){
	gf_program_eeprom(&board_eeprom,config);
}

/*
 * Upgrade current eeprom version to latest version
 * without data loss
 */
static void upgrade_eeprom_to_latest_version(struct eeprom* eep){
	char content[EEPROM_SPACE_USED];
	int i = 0;
	int found_matching_wid = 0;
	char * s;
	u8 checksum = 0;
	char checksum_string[3];

	gf_debug(0,"Upgrading Eeprom from GF00%d to GF003\n",eep->protocol.version);
	gf_memset(content,0xFF,sizeof(content));
	content[0]=0;
	/* Insert ID and Rev field for new eeprom version */
	gf_strcat(content,"GF003\n");
	gf_debug(2,"Current Eeprom GF00%d has %d fields\n",eep->protocol.version, eep->protocol.total_fields);
	switch(eep->protocol.version){
	/* Update from protocol version 0 */
	case 0:
		/* Find SW ID field from WID table using product code as key */
		for (i = 0; i < WID_TABLE_LENGTH; i++)
		{
			if(!gf_strncmp((eep->fields[0])+3,legacy_wid_translation_table[i].product_code,PRODUCT_CODE_LEN))
			{
				found_matching_wid = 1;
				break;
			}
		}
		if (found_matching_wid == 0)
		{
			gf_debug(0,"Eeprom Upgrade Failed. Product code unknown\n");
			return;
		}
		/* Add SW ID field to content*/
		gf_strcat(content,legacy_wid_translation_table[i].sw_id_code);
		gf_strcat(content,"\n");
		/* Add the remaining fields with the exception of checksum field */
		for (i = 0; i<eeprom_protocol_GF003.total_fields-2; i++)
		{
			/* If the corresponding field is found in the original GF000 eeprom
			 * copy the field from there. Else leave blank
			 */
			if(i<eep->protocol.total_fields)
			{
				gf_strcat(content,eep->fields[i]);
				gf_strcat(content,"\n");
			} else {
				gf_strcat(content,"\n");
			}
		}

		/* Start computing checksum after GF003 protocol header */
		s = content+EEPROM_ID_CODE_LEN+EEPROM_PROTOCOL_VERSION_LEN+1;
		/* Calculate checksum */
		for (i = 0; i<eeprom_protocol_GF003.total_fields-1;i++)
		{
			while (*s != '\n') {
				checksum = checksum + *(unsigned char*) s;
				s++;
			}
			s++;
		}
		checksum = checksum & 0XFF;
		/* Convert checksum to string */
		gf_u8tox(checksum,checksum_string);
		/* Append checksum to eeprom content */
		gf_strcat(content,checksum_string);
		gf_strcat(content,"\n");
		gf_debug(0,"Upgrading Eeprom...");
		/* Copy upgraded content to eeprom */
		gf_i2c_set_bus_num(eep->bus_num);
		if(gf_eeprom_write(eep->i2c_address,0,(unsigned char *) content,EEPROM_SPACE_USED))
		{
			gf_debug(0,"\nEeprom Upgrade Failed. Write error,\n");
			return;
		}
		gf_debug(0,"Done\n");
		return;
		break;
	default:
		return;
	}
}

void upgrade_som_eeprom_to_latest_version(void){
	upgrade_eeprom_to_latest_version(&som_eeprom);
}
void upgrade_board_eeprom_to_latest_version(void){
	upgrade_eeprom_to_latest_version(&board_eeprom);
}

/* Load eeprom contents in RAM */
void load_eeprom(struct eeprom* eep) {
	int n_current_field = 0;
	int ch_pos;
	int n_block;
	unsigned int expected_checksum = 0;
	unsigned int read_checksum;

	/* Check eeprom protocol version */
	switch (eep->protocol.version) {
	case -1:
		eep->status = EEPROM_NOT_PROGRAMMED;
		return;
	case 0:
		/* Legacy protocol version 0 eeproms */
		eep->protocol.total_fields = 4;
		break;
	case 1:
		eep->protocol.total_fields = eeprom_protocol_GF001.total_fields;
		break;
	case 2:
		eep->protocol.total_fields = eeprom_protocol_GF002.total_fields;
		break;
	case 3:
		eep->protocol.total_fields = eeprom_protocol_GF003.total_fields;
		break;
	default:
		gf_debug(0,"Eeprom Protocol Version is not supported in this SW version\n");
		eep->status = EEPROM_NOT_PROGRAMMED;
		return;
	}

	/* Read eeprom */
	for (n_block = 0;
			((n_current_field < eep->protocol.total_fields + 1)
					&& (n_block < EEPROM_MAX_N_BLOCKS)); n_block++) {
		gf_debug(2,"Eeprom Read Block %d\n",n_block);
		gf_eeprom_read(eep->i2c_address, n_block * EEPROM_BLOCK_LEN,
				(unsigned char *) &eep->content[n_block * EEPROM_BLOCK_LEN],
				EEPROM_BLOCK_LEN);
		/* In protocol version 0, the first field is the board product code */
		if (eep->protocol.version == 0 && n_block == 0)
		{
			eep->fields[n_current_field] = &eep->content[0];
			n_current_field++;
		}
		for (ch_pos = n_block * EEPROM_BLOCK_LEN;
				ch_pos < ((n_block + 1) * EEPROM_BLOCK_LEN); ch_pos++) {
			if (eep->content[ch_pos] == '\n') {
				eep->content[ch_pos] = 0;
				if (n_current_field != eep->protocol.total_fields)
					eep->fields[n_current_field] =
							&eep->content[ch_pos + 1];
				n_current_field = n_current_field + 1;
				eep->len = ch_pos;
			}
		}
	}
	gf_debug(2,"Eeprom length: %d\n",eep->len);
	gf_debug(2,"Eeprom found %d fields \n",n_current_field);
	/* Check if read number of fields matches with protocol version and test checksum
	 * Version 0 doesn't have a fixed number of fields nor checksum version verification
	 */
	if(eep->protocol.version > 0)
	{
		if (n_current_field != eep->protocol.total_fields + 1) {
			gf_debug(0,"Eeprom protocol mismatch\n");
			eep->status = EEPROM_NOT_PROGRAMMED;
			return;
		}

		expected_checksum = checksum(eep);

		read_checksum = gfstr_toul(eep->fields[eep->protocol.total_fields - 1], NULL, 16);
		if (expected_checksum != read_checksum) {
			gf_debug(0,"Eeprom Checksum Error: Read %d, Expected %d\n",
					read_checksum, expected_checksum);
			eep->status = EEPROM_NOT_PROGRAMMED;
			return;
		}

		gf_debug(0,"Eeprom Checksum validated\n");
	}
	else
	{
		if(eep->protocol.total_fields != n_current_field - 1 )
			eep->protocol.total_fields = 1;
	}

	for(n_current_field=0;n_current_field<eep->protocol.total_fields;n_current_field++)
	{
		gf_debug(2,"Eeprom Field %d: %s\n",n_current_field,eep->fields[n_current_field]);
	}

	eep->status = EEPROM_PROGRAMMED;
	return;
}

void load_som_eeprom(void)
{
	load_eeprom(&som_eeprom);
}

void load_board_eeprom(void)
{
	load_eeprom(&board_eeprom);
}

void init_gf_som_eeprom(void)
{
	/* Prepare SOM eeprom structure */
	init_eeprom(&som_eeprom, SOM_EEPROM_I2C_BUS_NO, SOM_EEPROM_I2C_ADDRESS, 2);
}
void init_gf_board_eeprom(void)
{
	/* Prepare SOM eeprom structure */
	init_eeprom(&board_eeprom, BOARD_EEPROM_I2C_BUS_NO, BOARD_EEPROM_I2C_ADDRESS, 2);
}

int gf_load_som_revision(char ** egf_sw_id_code, int bypass_checks)
{
	int som_eeprom_hw_status;
	int som_eeprom_protocol;
	char * serial_number;
	int ret;
	int index = 0;
	int eeprom_is_empty;
	char c;

	gf_debug(0,"Eeprom GF module version: %s\n",GF_EEPROM_SW_VERSION);
	gf_i2c_init();
	init_gf_som_eeprom();
	/* Detect SOM eeprom */
	som_eeprom_hw_status = check_eeprom_hw_som();
	if (som_eeprom_hw_status == EEPROM_NOT_FOUND)
	{
		/* If SOM eeprom is not found stop boot */
		gf_debug(0,"Eeprom not detected. HW issue?\n");
		return -1;
	}
	eeprom_is_empty = som_eeprom_is_new();
	if(eeprom_is_empty != 1)
	{
		/* Try to detect SOM eeprom protocol version */
		som_eeprom_protocol = identify_eeprom_protocol_som();
		if (som_eeprom_protocol != EEPROM_UNKNOWN_PROTOCOL)
		{
			/* Read SOM eeprom */
			load_som_eeprom();
			/* If SOM eeprom is programmed with an old protocol version update it */
			if (bypass_checks == 0 &&
				som_eeprom_protocol != EEPROM_UNKNOWN_PROTOCOL &&
				som_eeprom_protocol < EEPROM_LATEST_PROTOCOL_VERSION)
			{
				/* SOM eeprom upgrade needed */
				gf_som_eeprom_unlock();
				upgrade_som_eeprom_to_latest_version();
				gf_som_eeprom_lock();
				/* Re-load SOM eeprom after updating */
				som_eeprom_protocol = identify_eeprom_protocol_som();
				load_som_eeprom();
			}
			/* Get SW ID code and serial number stored in SOM eeprom */
			*egf_sw_id_code = gf_eeprom_get_som_sw_id_code();
			serial_number = gf_eeprom_get_som_serial_number();
			if (serial_number != NULL) {
				gf_debug(1,"Board Serial Number: %s\n",serial_number);
			}
			/* If SW ID is programmed, eeprom is validated */
			if (*egf_sw_id_code != NULL)
			{
				return 0;
			}
			gf_debug(0,"Invalid GF Software ID code\n");
		}

		if (bypass_checks != 0)
			return -1;
		/* Ask user confirmation to reprogram eeprom */
		gf_serial_init();
		gf_debug(0,"Eeprom corrupted.Reprogram now? [Type y to confirm] ");
		c = gf_serial_getc();
		gf_debug(0,"%c\n",c);
		if(c != 'y' && c != 'Y')
			return -1;
	}

	/* Check if eeprom programmer file is available on microSD */
	ret = gf_read_programmer_file(EEPROM_PROGRAMMER_FILE_NAME,gf_file_buffer,EEPROM_PROGRAMMER_FILE_BUFFER_SIZE);
	if (ret == -1)
	{
		return -1;
	}
	gf_debug(0,"GF Eeprom programmer file found. Entering EEPROM programming mode\n");
	gf_debug(2,"Read from Eeprom programmer file: %s",gf_file_buffer);
	/* Programmer file format <WID>;<PC>;
	 * example: WID0336_AA01.00;JSC0336_D01.Beta1_Conf-001_A01;
	 */
	while (gf_file_buffer[index]!=';' && index < WID_STR_LEN )
	{
		read_config.wid[index] = gf_file_buffer[index];
		index++;
	}
	read_config.wid[WID_STR_LEN] = 0;

	index = 0;
	while (gf_file_buffer[index + WID_STR_LEN + 1]!=';'
			&& index < PC_STR_LEN )
	{
		read_config.product_code[index] = gf_file_buffer[index + WID_STR_LEN + 1];
		index++;
	}
	read_config.product_code[index]=0;

	gf_debug(2,"Read WID: %s\n",read_config.wid);
	gf_debug(2,"Read PC: %s\n",read_config.product_code);

	/* Write data read from file to SOM eeprom */
	gf_som_eeprom_unlock();
	program_som_eeprom(&read_config);
	gf_som_eeprom_lock();

	/* Re-load SOM eeprom after programming */
	som_eeprom_protocol = identify_eeprom_protocol_som();
	load_som_eeprom();
	*egf_sw_id_code = gf_eeprom_get_som_sw_id_code();
	/* If SW ID is programmed, eeprom is validated */
	if (*egf_sw_id_code != NULL)
	{
		return 0;
	}
	return -1;

}


