/*
* http://download.mikroe.com/documents/datasheets/R503_datasheet.pdf
*/

#ifndef __FINGERPRINT_H__
#define __FINGERPRINT_H__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pigpio.h>


/* R503 fingerprint sensor macros */
#define R503_MSG_HEADER 0xEF01
#define R503_DEFAULT_ADDRESS 0xFFFFFFFF

/* R503 fingerprint sensor packet types */
#define R503_PACKET_CMD 0x01
#define R503_PACKET_DATA 0x02
#define R503_PACKET_ACK 0x07
#define R503_PACKET_EOD 0x08

/* R503 fingerprint sensor instruction codes */
#define R503_INSTR_GET_FINGER_IMG 0x01
#define R503_INSTR_IMG_TO_CHAR_FILE 0x02
#define R503_INSTR_MATCH_TEMPLATES 0x03
#define R503_INSTR_SEARCH_FINGER_LIB 0x04
#define R503_INSTR_GEN_TEMPLATE 0x05
#define R503_INSTR_STORE_TEMPLATE 0x06
#define R503_INSTR_READ_TEMPLATE 0x07
#define R503_INSTR_UPLOAD_TEMPLATE 0x08
#define R503_INSTR_DOWNLOAD_TEMPLATE 0x09
#define R503_INSTR_UPLOAD_IMG 0x0A
#define R503_INSTR_DOWNLOAD_IMG 0x0B
#define R503_INSTR_DELETE_TEMPLATE 0x0C
#define R503_INSTR_READ_TEMPLATE_IDX_TAB 0x1F
#define R503_INSTR_CANCEL_INSTR 0x30
#define R503_INSTR_CHECK_SENSOR 0x36
#define R503_INSTR_GET_FW_VER 0x3A
#define R503_INSTR_SOFT_RESET 0x3D
#define R503_INSTR_EMPTY_LIB 0x0D
#define R503_INSTR_SET_SYS_PARAM 0x0E
#define R503_INSTR_READ_SYS_PARAM 0x0F
#define R503_INSTR_SET_PASSWD 0x12
#define R503_INSTR_VERIFY_PASSWD 0x13
#define R503_INSTR_GET_RAND_CODE 0x14
#define R503_INSTR_SET_DEV_ADDR 0x15
#define R503_INSTR_READ_INFO_PAGE 0x16
#define R503_INSTR_PORT_CTRL 0x17
#define R503_INSTR_WRITE_NOTEPAD 0x18
#define R503_INSTR_READ_NOTEOPAD 0x19
#define R503_INSTR_READ_TEMPLATE_NUMS 0x1D
#define R503_INSTR_GET_IMG_EX 0x28
#define R503_INSTR_HANDSHAKE 0x40
#define R503_INSTR_GET_ALG_LIB_VER 0x39
#define R503_INSTR_READ_PROD_INFO 0x3C
#define R503_INSTR_LED_CONFIG 0x35

/* R530 fingerprint sensor acknowledge packet codes */
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

/* R503 fingerprint sensor LED macros */
#define R503_LED_BREATHING 0x01
#define R503_LED_FLASHING 0x02
#define R503_LED_ON 0x03
#define R503_LED_OFF 0x04
#define R503_LED_SLOW_ON 0x05
#define R503_LED_SLOW_OFF 0x06

#define R503_LED_RED 0x01
#define R503_LED_BLUE 0x02
#define R503_LED_PURPLE 0x03

/* signal hadling macros */
#define SIGNAL_TERMINATE (unsigned)2

/* serial port handling macros */
#define UART_PORT_NAME "/dev/ttyAMA0"
#define UART_BAUD_RATE (unsigned)115200
#define DATA_WAIT_TIMEOUT_MICROS (uint32_t)1000000

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
	EBadAlloc = -1,
	ENullPtr = -2,
	EGpioBadInit = -10,
	EGpioBadMode = -11,
	EGpioBadPud = -12,
	EGpioBadISR = -13,
	ETtyBadOpen = -20,
	ETtyBadWrite = -21,
	ETtyBadHandle = -22,
	ETtyTimeout = -23,
	EOk = 0
	};

typedef struct
	{
	uint16_t header;
	uint32_t address;
	uint8_t package_id;
	uint16_t package_length;
	uint16_t checksum;
	size_t send_packet_length;
	int serial_handle;

	uint8_t *data;
	uint8_t *send_packet;
	} fp_packet_r503;


void HandleFinger(int aGpio, int aLevel, uint32_t aTick, void * aData);
void HandleSignal(int aSignum, void * aData);
int GpioConfig(int *aIsrData, int *aSigData);

int BufferAlloc(uint8_t ** const aBuffer, const size_t aLen);
void BufferDealloc(uint8_t ** aBuffer);
int CtorFpPacket(fp_packet_r503 * const aStruct, const uint8_t aId, const uint16_t aLen, const uint8_t * const aData, const int * const aHandle);
void DtorFpPacket(fp_packet_r503 * aStruct);
int SendFpPacket(const fp_packet_r503 * const aPacket);
int ReadFpPacket(fp_packet_r503 * const aPacket);
int WaitForData(const int aSerHandle);
int64_t ReadByBytes(const int aSerHandle, unsigned aNumofBytes);
void PrintFpPacket(const fp_packet_r503 * const aPacket, const char * const aPcktType);
int SetFpLed(const int * const aSerHandle, const uint8_t aState, const uint8_t aColor, const uint8_t aPeriod, const uint8_t aCount);
int GetFpResponse();
int GetFingerImg(const int * const aSerHandle, const int * const aKillSig);
int FingerImgToBuffer(const int * const aSerHandle, int aBuffNum);
int GenFingerTemplate(const int * const aSerHandle, const int * const aKillSig);

#endif /* __FINGERPRINT_H__ */
