#include "fingerprint.h"


void HandleFinger(int aGpio, int aLevel, uint32_t aTick, void * aData)
	{
	fprintf(stdout, "finger pressed\n");
	}

void HandleSignal(int aSignum, void * aData)
	{
	*(int*)aData = TRUE;
	}


int GpioConfig(int *aIsrData, int *aSigData)
	{
	if (gpioInitialise() < 0)
		{
		fprintf(stderr, "\n%s: ERROR! Could not initialise pigpio library.\n", __argv[0]);
		return EGpioBadInit;
		}

	if (gpioSetMode(GPIO_FINGER_WAKEUP, PI_INPUT) == PI_BAD_GPIO)
		{
		fprintf(stderr, "\n%s: ERROR! Could not set input mode on GPIO %d.\n", __argv[0], GPIO_FINGER_WAKEUP);
		return EGpioBadMode;
		}

	if (gpioSetPullUpDown(GPIO_FINGER_WAKEUP, PI_PUD_OFF))
		{
		fprintf(stderr, "\n%s: ERROR! Could not set off pull up/down on GPIO %d.\n", __argv[0], GPIO_FINGER_WAKEUP);
		return EGpioBadPud;
		}

	if (aSigData)
		{
		if (gpioSetSignalFuncEx(SIGNAL_TERMINATE, HandleSignal, (void*)aSigData))
			{
			fprintf(stderr, "\n%s: ERROR! Could not set signal function on signal %d.\n", __argv[0], SIGNAL_TERMINATE);
			return EGpioBadISR;
			}
		}

	if (aIsrData)
		{
		if(gpioSetISRFuncEx(GPIO_FINGER_WAKEUP, ISR_DETECTION_LEVEL, 0, HandleFinger, (void*)aIsrData))
			{
			fprintf(stderr, "\n%s: ERROR! Could not set ISR function on GPIO %d.\n", __argv[0], GPIO_FINGER_WAKEUP);
			return EGpioBadISR;
			}
		}

	return EOk;
	}

int BufferAlloc(uint8_t ** const aBuffer, const size_t aLen)
	{
	*aBuffer = (uint8_t*)malloc(aLen * sizeof(uint8_t));
	if (!*aBuffer)
		return EBadAlloc;
	memset(*aBuffer, 0, aLen);

	return EOk;
	}

void BufferDealloc(uint8_t ** aBuffer)
	{
	if (aBuffer)
		{
		free(*aBuffer);
		*aBuffer = NULL;
		}
	}

int CtorFpPacket(fp_packet_r503 * const aStruct, const uint8_t aId, const uint16_t aLen, const uint8_t * const aData, const int * const aHandle)
	{
	if (!aStruct)
		return ENullPtr;

	aStruct->header = R503_MSG_HEADER;
	aStruct->address = R503_DEFAULT_ADDRESS;
	aStruct->package_id = aId;
	aStruct->package_length = aLen;

	if (BufferAlloc(&aStruct->data, aLen - sizeof(aStruct->checksum)))
		return EBadAlloc;
	memcpy(aStruct->data, aData, aLen - sizeof(aStruct->checksum));

	//sum -> over each single byte
	//should work, if not try summing whole 16bit length instead of separating to 2 8bit values
	aStruct->checksum = aStruct->package_id + (uint8_t)(aStruct->package_length >> 8) + (uint8_t)(aStruct->package_length & 0xFF);
	for (size_t data_byte = 0; data_byte < aStruct->package_length - 2; ++data_byte)
		aStruct->checksum += aStruct->data[data_byte];

	aStruct->send_packet_length = sizeof(aStruct->header) + sizeof(aStruct->address) + sizeof(aStruct->package_id) + sizeof(aStruct->package_length) + aStruct->package_length;
	if (BufferAlloc(&aStruct->send_packet, aStruct->send_packet_length))
		return EBadAlloc;
	aStruct->send_packet[0] = aStruct->header >> 8;
	aStruct->send_packet[1] = aStruct->header & 0xFF;
	aStruct->send_packet[2] = aStruct->address >> 24;
	aStruct->send_packet[3] = (aStruct->address >> 16) & 0xFF;
	aStruct->send_packet[4] = (aStruct->address >> 8) & 0xFF;
	aStruct->send_packet[5] = aStruct->address & 0xFF;
	aStruct->send_packet[6] = aStruct->package_id;
	aStruct->send_packet[7] = aStruct->package_length >> 8;
	aStruct->send_packet[8] = aStruct->package_length & 0xFF;
	for (size_t data_byte = 0; data_byte < aStruct->package_length; ++data_byte)
		aStruct->send_packet[9 + data_byte] = aStruct->data[data_byte];
	aStruct->send_packet[aStruct->send_packet_length - 2] = aStruct->checksum >> 8;
	aStruct->send_packet[aStruct->send_packet_length - 1] = aStruct->checksum & 0xFF;

	if (aHandle)
		aStruct->serial_handle = *aHandle;
	else
		aStruct->serial_handle = 0;

	return EOk;
	}

void DtorFpPacket(fp_packet_r503 * aStruct)
	{
	if (!aStruct)
		{
		aStruct->header = 0;
		aStruct->address = 0;
		aStruct->package_id = 0;
		aStruct->package_length = 0;
		aStruct->checksum = 0;
		aStruct->send_packet_length = 0;

		BufferDealloc(&aStruct->data);
	        BufferDealloc(&aStruct->send_packet);
		}
	}

int GetFingerImg()
	{
	

	return EOk;
	}

int SendFpPacket(const fp_packet_r503 * const aPacket)
	{
	for (size_t packet_byte = 0; packet_byte < aPacket->send_packet_length; ++packet_byte)
		{
		if (serWriteByte(aPacket->serial_handle, aPacket->send_packet[packet_byte]))
			{
			fprintf(stderr, "\n%s: ERROR! Could not send %dth byte to serial port %s.\n", __argv[0], packet_byte, UART_PORT_NAME);
			return ETtyBadWrite;
			}
		}

	return EOk;
	}

int SetFpLed(const int * const aSerHandle, const uint8_t aState, const uint8_t aColor, const uint8_t aPeriod, const uint8_t aCount)
	{
	fp_packet_r503 led_pckt = { 0, };
	uint8_t data[5] = { R503_INSTR_LED_CONFIG, aState, aPeriod, aColor, aCount };
	if (CtorFpPacket(&led_pckt, 0x1, 0x7, data, aSerHandle))
		{
		fprintf(stderr, "\n%s: ERROR! Could not create packet.\n", __argv[0]);
		return EBadAlloc; //maybe change
		}

	if (SendFpPacket(&led_pckt))
		return EBadAlloc; //change return value

	if (GetFpResponse())
		return EBadAlloc; //change return value

	return EOk;
	}

int ReadFpPacket(fp_packet_r503 * const aPacket)//TODO: timeout for waiting and separate function
	{
	if (serDataAvailable(aPacket->serial_handle) == PI_BAD_HANDLE)
		return ETtyBadHandle;

	while (serDataAvailable(aPacket->serial_handle) == 0) {}
	aPacket->header = serReadByte(aPacket->serial_handle);
	aPacket->header <<= 8;
	while (serDataAvailable(aPacket->serial_handle) == 0) {}
	aPacket->header |= serReadByte(aPacket->serial_handle);

	while (serDataAvailable(aPacket->serial_handle) == 0) {}
	aPacket->address = serReadByte(aPacket->serial_handle);
	aPacket->address <<= 8;
	while (serDataAvailable(aPacket->serial_handle) == 0) {}
	aPacket->address |= serReadByte(aPacket->serial_handle);
	aPacket->address <<= 8;
	while (serDataAvailable(aPacket->serial_handle) == 0) {}
	aPacket->address |= serReadByte(aPacket->serial_handle);
	aPacket->address <<= 8;
	while (serDataAvailable(aPacket->serial_handle) == 0) {}
	aPacket->address |= serReadByte(aPacket->serial_handle);

	while (serDataAvailable(aPacket->serial_handle) == 0) {}
	aPacket->package_id = serReadByte(aPacket->serial_handle);

	while (serDataAvailable(aPacket->serial_handle) == 0) {}
	aPacket->package_length = serReadByte(aPacket->serial_handle);
	aPacket->package_length <<= 8;
	while (serDataAvailable(aPacket->serial_handle) == 0) {}
	aPacket->package_length |= serReadByte(aPacket->serial_handle);

	if (BufferAlloc(&aPacket->data, aPacket->package_length - sizeof(aPacket->checksum)))
		return EBadAlloc;
	for (size_t data_byte = 0; data_byte < aPacket->package_length - 2; ++data_byte)
		{
		while (serDataAvailable(aPacket->serial_handle) == 0) {}
		aPacket->data[data_byte] = serReadByte(aPacket->serial_handle);
		}

	while (serDataAvailable(aPacket->serial_handle) == 0) {}
	aPacket->checksum = serReadByte(aPacket->serial_handle);
	aPacket->checksum <<= 8;
	while (serDataAvailable(aPacket->serial_handle) == 0) {}
	aPacket->checksum |= serReadByte(aPacket->serial_handle);


	/*serRead(aPacket->serial_handle, (char*)&aPacket->header, 2);
	aPacket->header = ((aPacket->header & (uint16_t)0xFF) << 8) | ((aPacket->header & 0xFF00) >> 8);*/

	return EOk;
	}

void PrintFpPacket(const fp_packet_r503 * const aPacket, const char * const aPcktType)
	{
	if (aPcktType)
		fprintf(stdout, "%s: %s packet contents:\n", __argv[0], aPcktType);
	else
		fprintf(stdout, "%s: Packet contents:\n", __argv[0]);

	fprintf(stdout, "%s: headder: 0x%x\n", __argv[0], aPacket->header);
	fprintf(stdout, "%s: address: 0x%x\n", __argv[0], aPacket->address);
	fprintf(stdout, "%s: package ID: 0x%x\n", __argv[0], aPacket->package_id);
	fprintf(stdout, "%s: package length: 0x%x\n", __argv[0], aPacket->package_length);
	fprintf(stdout, "%s: data:\n", __argv[0]);
	for (size_t data_byte = 0; data_byte < aPacket->package_length - 2; ++data_byte)
		fprintf(stdout, "%s:  0x%x\n", __argv[0], aPacket->data[data_byte]);
	fprintf(stdout, "%s: checksum: 0x%x\n", __argv[0], aPacket->checksum);
	}

int GetFpResponse()
	{
	fp_packet_r503 ack_pckt = { 0, };
	ReadFpPacket(&ack_pckt);
	int out = ack_pckt.data[0];
	DtorFpPacket(&ack_pckt);

	return out;
	}
