#include "fingerprint.h"


int __argc = 0;
char ** __argv = NULL;

int main(int argc, char ** argv)
	{
	__argc = argc;
	__argv = argv;

	int terminate = FALSE, isr_uselater = 0, serhandle = 0;

	if(GpioConfig(&isr_uselater, &terminate))
		return EGpioBadInit;

	if ((serhandle = serOpen(UART_PORT_NAME, UART_BAUD_RATE, 0)) < 0)
		{
		fprintf(stderr, "\n%s: ERROR! Could not open serial port %s (handle %d).\n", __argv[0], UART_PORT_NAME, serhandle);
		return ETtyBadOpen;
		}

	uint8_t pid = 0x1;
	uint16_t len = 0x3;
	uint8_t pdata[1] = { 0xF };
	fp_packet_r503 packet = { 0, };
	if (CtorFpPacket(&packet, pid, len, pdata, &serhandle))
		fprintf(stderr, "\n%s: ERROR! Could not create R503 sensor packet.\n", __argv[0]);

	if (SendFpPacket(&packet))
		fprintf(stderr, "\n%s: ERROR! Could not sent full packet.\n", __argv[0]);

	fp_packet_r503 rec_packet = { 0, };
	ReadFpPacket(&rec_packet);

	PrintFpPacket(&rec_packet, "Received");

	SetFpLed(&serhandle, R503_LED_FLASHING, R503_LED_PURPLE, 0x20, 0x10);

	while (!terminate)
		{}

	DtorFpPacket(&packet);
	DtorFpPacket(&rec_packet);
	serClose(serhandle);
	return EOk;
	}
