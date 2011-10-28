#ifndef _REVISION_CODES_H_
#define _REVISION_CODES_H_
/* SOM CODE */
#define REV_STR_TO_REV_CODE(REV_STRING) \
	(\
	(((REV_STRING[3]-'0')*1000 + (REV_STRING[4]-'0')*100+(REV_STRING[5]-'0')*10 + (REV_STRING[6]-'0')) << 16)|\
	((REV_STRING[8]-'A') << 8)|\
	((REV_STRING[9]-'0')*10 + (REV_STRING[10]-'0'))\
	)

#define SOM_REV_CODE(REV1,REV2,REV3)\
	((REV1<<16) | ((REV2-'A') << 8) |  REV3)

#define REV_NOT_PROGRAMMED  SOM_REV_CODE(((0xFF-'0')*1000 + (0xFF-'0')*100+(0xFF-'0')*10 + 0xff-'0'),'A',0xFF)

#define N_REVISIONS	3
char* revision_strings[N_REVISIONS]={
		"JSF0385_A01",
		"JSF0385_B01",
		"JSF0385_C01"
};


#define REV_385_A01  SOM_REV_CODE(385,'A',1)
#define REV_385_B01  SOM_REV_CODE(385,'B',1)
#define REV_385_C01  SOM_REV_CODE(385,'C',1)

#define SOM_REVISION_LEN  12  /* termination character included. ex: JSC0336_A02*/

#endif
