#include "fingerprint.h"


int __argc = 0;
char ** __argv = NULL;

int main(int argc, char ** argv)
	{
	__argc = argc;
	__argv = argv;

	int Err = 0, terminate = FALSE, isr_uselater = 0, serhandle = 0;

	if(GpioConfig(&isr_uselater, &terminate))
		return EGpioBadInit;

	if ((serhandle = serOpen(UART_PORT_NAME, UART_BAUD_RATE, 0)) < 0)
		{
		fprintf(stderr, "\n%s: ERROR! Could not open serial port %s (handle %d).\n", __argv[0], UART_PORT_NAME, serhandle);
		return ETtyBadOpen;
		}

	uint8_t pid = R503_PACKET_CMD;
	uint16_t len = 0x3;
	uint8_t pdata[1] = { R503_INSTR_READ_SYS_PARAM };
	fp_packet_r503 packet = { 0, };
	if ((Err = CtorFpPacket(&packet, pid, len, pdata, &serhandle)))
		fprintf(stderr, "\n%s: ERROR! Could not create R503 sensor packet.\n", __argv[0]);

	if ((Err = SendFpPacket(&packet)))
		fprintf(stderr, "\n%s: ERROR! Could not send full packet.\n", __argv[0]);

	fp_packet_r503 rec_packet = { 0, };
	if ((Err = ReadFpPacket(&rec_packet)))
		fprintf(stderr, "\n%s: ERROR! Could not read full ack packet.\n", __argv[0]);

	if (!Err)
		PrintFpPacket(&rec_packet, "Received");

	SetFpLed(&serhandle, R503_LED_FLASHING, R503_LED_PURPLE, 0x20, 0x10);

	gpioSetISRFuncEx(GPIO_FINGER_WAKEUP, ISR_DETECTION_LEVEL, 0, NULL, NULL);
	GenFingerTemplate(&serhandle, &terminate);
	gpioDelay((uint32_t)1000000);
	gpioSetISRFuncEx(GPIO_FINGER_WAKEUP, ISR_DETECTION_LEVEL, 0, HandleFinger, (void*)&isr_uselater);

	while (!terminate)
		{}

	DtorFpPacket(&packet);
	DtorFpPacket(&rec_packet);
	serClose(serhandle);
	return EOk;
	}
