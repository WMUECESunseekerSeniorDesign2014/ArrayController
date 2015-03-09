#ifndef RS232_H_
#define RS232_H_

/*
 * Constant Declarations
 */
static char RS232Cmd[5] = "+++\r\0";
static char RS232_Test1[13] = "Sunseeker \n\r\0";
static char RS232_Test2[9] = "2014. \n\r\0";
static char Parse_header[6][5] = {"BJB \0","JLB \0","AGC \0","JSD \0","JTL \0","SPI \0"};

/**
 * @defgroup promptDefs Prompt Defines
 * @{
 */
#define PROMPT_EXIT 			0
#define PROMPT_BATT_DUMP 		1
#define PROMPT_SHUNT_DUMP 		2
#define PROMPT_MPPT_DUMP 		3
#define PROMPT_THERM_DUMP 		4
#define PROMPT_MPPT_SWITCH_DUMP 5
/** @} */
/*
 * Function Definitions
 */
// Internal RS-232 Function Definitions
void AC2GPS_init(void);
void AC2GPS_putchar(char data);
unsigned char AC2GPS_getchar(void);
int AC2GPS_puts(char *str);
int AC2GPS_gets(char *ptr);
void AC2GPS_put_int(void);

// External RS-232 Function Definitions
void AC2PC_init(void);
void AC2PC_putchar(char data);
unsigned char AC2PC_getchar(void);
int AC2PC_puts(char *str);
int AC2PC_gets(char *ptr);
void AC2PC_put_int(void);

#endif /* RS232_H_ */
