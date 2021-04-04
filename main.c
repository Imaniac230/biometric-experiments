#include "fingerprint.h"


int __argc = 0;
char ** __argv = NULL;

int main(int argc, char ** argv)
	{
	__argc = argc;
	__argv = argv;

	int terminate = FALSE, isr_uselater = 0, serhandle = 0, bytes_available = 0;

	if(GpioConfig(&isr_uselater, &terminate))
		return EGpioBadInit;

	if ((serhandle = serOpen(UART_PORT_NAME, UART_BAUD_RATE, 0)) < 0)
		{
		fprintf(stderr, "\n%s: ERROR! Could not open serial port.\n", __argv[0]);
		return ETtyBadOpen;
		}


/*	uint8_t headder[2] = { 0xEF, 0x01 };
	uint8_t address[4] = { 0xFF, 0xFF, 0xFF, 0xFF};
	uint8_t identifier = 0x01;
	uint8_t length[2] = { 0x00, 0x04 };
	uint8_t code = 0x19;
	uint8_t data = 0;
	uint16_t sum = identifier + length[0] + length[1] + data + code;*/

	uint8_t pid = 0x01;
	uint16_t len = 0x0004;
	uint8_t pdata[2] = { 0x19, 0x00 };
	fp_packet_r503 packet = { 0, };
        if (CtorFpPacket(&packet, pid, len, pdata))
                fprintf(stderr, "error creating packet");

	/*printf("header: 0x%x\n", packet.header);
	printf("address: 0x%x\n", packet.address);
	printf("package id: 0x%x\n", packet.pckg_id);
	printf("package length: 0x%x\n", packet.pckg_len);
	for (size_t i = 0; i < packet.pckg_len - 2; ++i)
		printf("data byte %d: 0x%x\n", i, packet.data[i]);
	printf("checksum: 0x%x\n", packet.chcksum);
	printf("correct checksum: 0x%x\n", sum);*/

	for (size_t i = 0; i < packet.full_pckt_len; ++i)
		{
		if (serWriteByte(serhandle, packet.full_pckt[i]))
			fprintf(stderr, "error sending to serial\n");
		}

	/*if (serWriteByte(serhandle, headder[0]))
		fprintf(stderr, "ERROR sending to serial\n");
	if (serWriteByte(serhandle, headder[1]))
		fprintf(stderr, "ERROR sending to serial\n");

	if (serWriteByte(serhandle, address[0]))
		fprintf(stderr, "ERROR sending to serial\n");
	if (serWriteByte(serhandle, address[1]))
		fprintf(stderr, "ERROR sending to serial\n");
	if (serWriteByte(serhandle, address[2]))
		fprintf(stderr, "ERROR sending to serial\n");
	if (serWriteByte(serhandle, address[3]))
		fprintf(stderr, "ERROR sending to serial\n");

	if (serWriteByte(serhandle, identifier))
		fprintf(stderr, "ERROR sending to serial\n");

	if (serWriteByte(serhandle, length[0]))
		fprintf(stderr, "ERROR sending to serial\n");
	if (serWriteByte(serhandle, length[1]))
		fprintf(stderr, "ERROR sending to serial\n");

	if (serWriteByte(serhandle, code))
		fprintf(stderr, "ERROR sending to serial\n");

	if (serWriteByte(serhandle, data))
		fprintf(stderr, "ERROR sending to serial\n");

	if (serWriteByte(serhandle, (unsigned)(sum>>8)))
		fprintf(stderr, "ERROR sending to serial\n");
	if (serWriteByte(serhandle, (unsigned)(sum&0xFF)))
		fprintf(stderr, "ERROR sending to serial\n");*/

	//uint16_t a = 0x7E01;
	//printf("sum: 0x%x, sumH: 0x%x, sumL: 0x%x\n", a, (unsigned)(a>>8), (unsigned)(a&0xFF));
	//printf("sizeof uint16_t: %d\n", sizeof(a));

	while (!terminate)
		{
		if ((bytes_available = serDataAvailable(serhandle)) > 0)
			{
			//fprintf(stdout, "bytes_available = %d", bytes_available);
			printf("response: 0x%x\n", serReadByte(serhandle));
			}
		}


	serClose(serhandle);
	return EOk;
	}
