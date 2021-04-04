#ifndef __FINGERPRINT_H__
#define __FINGERPRINT_H__


#include <stdio.h>
//#include <sys/time.h>
//#include <stdint.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>

#include <pigpio.h>


/* R503 fingerprint sensor macros */
#define FINGERPRINT_MSG_HEADER 0xEF01

/* acknowledge packet codes */
#define R503_ACK_OK 0x00
#define R503_ACK_ERR_REC_DATA_PCKG 0x01
#define R503_ACK_ERR_NO_FINGER 0x02
#define R503_ACK_ERR_ENROLL 0x03
#define R503_ACK_ERR_DISORDERLY_FP_IMG 0x06
#define R503_ACK_ERR_SMALL_FP_IMG 0x07
#define R503_ACK_ERR_NO_FP_MATCH 0x08
#define R503_ACK_ERR_FP_MATCH__NOT_FOUND 0x09
#define R503_ACK_ERR_COMBINE_FILES 0x0A
#define R503_ACK_ERR_ID_OVERFLOW 0x0B
#define R503_ACK_ERR_TEMPLATE_READ 0x0C
#define R503_ACK_ERR_TEMPLATE_UPLOAD 0x0D
#define R503_ACK_ERR_REC_MULTIPLE_DATA_PCKGS 0x0E
#define R503_ACK_ERR_UPLOAD_IMG 0x0F
#define R503_ACK_ERR_DELETE_TEMPLATE 0x10
#define R503_ACK_ERR_CLEAR_FP_LIB 0x11
#define R503_ACK_ERR_BAD_PASSWORD 0x13
#define R503_ACK_ERR_INVALID_PRIMARY_IMG 0x15
#define R503_ACK_ERR_FLASH_WRITE 0x18
#define R503_ACK_ERR_NO_DEFINITION 0x19
#define R503_ACK_ERR_INVALID_REG_NUM 0x1A
#define R503_ACK_ERR_BAD_REG_CONFIG 0x1B
#define R503_ACK_ERR_NOTEPAD_PAGE_NUM 0x1C
#define R503_ACK_ERR_COMM 0x1D

/* signal hadling macros */
#define SIGNAL_TERMINATE (unsigned)2

/* serial port handling macros */
#define UART_PORT_NAME "/dev/ttyAMA0"
#define UART_BAUD_RATE (unsigned)57600

/* GPIO pin function macros */
#define GPIO_FINGER_WAKEUP (unsigned)22
#define GPIO_WAKEUP_STATE_DEFAULT (unsigned)1
#define ISR_DETECTION_LEVEL ((GPIO_WAKEUP_STATE_DEFAULT) ? FALLING_EDGE : RISING_EDGE)

/* boolean state macros */
#define TRUE (int)1
#define FALSE (int)0

/* white character definition macros */
#define TAB 9
#define SPACE 32
#define NEWLINE 10


extern int __argc;
extern char ** __argv;

enum TError
	{
	EWrongArgs = -1,
	EBadAlloc = -2,
	EBadScan = -3,
	EBadOpen = -4,
	EBadRead = -5,
	ENullPtr = -6,
	EBadWrite = -7,
	EBadSocket = -8,
	EBadServer = -9,
	EBadConnect = -10,
	EBadNtp = -11,
	EGpioBadInit = -12,
	EGpioBadMode = -13,
	EGpioBadPud = -14,
	EGpioBadWrite = -15,
	EGpioBadISR = -16,
	EBadThread = -17,
	ETtyBadOpen = -18,
	EOk = 0
	};

typedef struct
	{
	uint16_t header;
	uint32_t address;
	uint8_t pckg_id;
	uint16_t pckg_len;
	uint16_t chcksum;

	uint8_t *data;

	size_t full_pckt_len;
	uint8_t *full_pckt;
	} fp_packet_r503;


void HandleSignal(int aSignum, void * aData);
int GpioConfig(int *aIsrData, int *aSigData);

int CtorFpPacket(fp_packet_r503 * const aStruct, const uint8_t aID, const uint16_t aLen, uint8_t * aData);

#endif /* __FINGERPRINT_H__ */
