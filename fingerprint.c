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
		{
		DtorFpPacket(aStruct);
		return EBadAlloc;
		}
	memcpy(aStruct->data, aData, aLen - sizeof(aStruct->checksum));

	aStruct->checksum = aStruct->package_id + (uint8_t)(aStruct->package_length >> 8) + (uint8_t)(aStruct->package_length & 0xFF);
	for (size_t data_byte = 0; data_byte < aStruct->package_length - 2; ++data_byte)
		aStruct->checksum += aStruct->data[data_byte];

	aStruct->send_packet_length = sizeof(aStruct->header) + sizeof(aStruct->address) + sizeof(aStruct->package_id) + sizeof(aStruct->package_length) + aStruct->package_length;
	if (BufferAlloc(&aStruct->send_packet, aStruct->send_packet_length))
		{
		DtorFpPacket(aStruct);
		return EBadAlloc;
		}
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

int GetFingerImg(const int * const aSerHandle, const int * const aKillSig)
	{
	if (!aSerHandle)
		return ENullPtr;

	fp_packet_r503 finger_pckt = { 0, };
	uint8_t data[1] = { R503_INSTR_GET_FINGER_IMG };
	int Err = 0, terminate = FALSE;
	if ((Err = CtorFpPacket(&finger_pckt, R503_PACKET_CMD, 0x3, data, aSerHandle)))
		{
		fprintf(stderr, "\n%s: ERROR! Could not create packet for finger IMG storage command.\n", __argv[0]);
		return Err;
		}

	fprintf(stdout, "\n%s: Place finger on sensor for detection ...\n", __argv[0]);
	Err = R503_ACK_ERR_NO_FINGER;
	while ((Err != R503_ACK_OK) && !terminate)
		{
		if ((Err = SendFpPacket(&finger_pckt)))
			{
			DtorFpPacket(&finger_pckt);
			return Err;
			}

		Err = GetFpResponse();
		if ((Err != R503_ACK_OK) && (Err != R503_ACK_ERR_NO_FINGER))
			{
			fprintf(stderr, "\n%s: ERROR! Error when detecting finger.\n", __argv[0]);
			DtorFpPacket(&finger_pckt);
			return Err;
			}

		if (aKillSig && *aKillSig)
			terminate = TRUE;
		}
	if (!aKillSig || (aKillSig && !*aKillSig))
		fprintf(stdout, "\n%s: Finger image stored successfully.\n\n", __argv[0]);

	DtorFpPacket(&finger_pckt);
	return EOk;
	}

int FingerImgToBuffer(const int * const aSerHandle, int aBuffNum)
	{
	if (!aSerHandle)
		return ENullPtr;

	int buff_num = aBuffNum;
	if ((aBuffNum != 1) && (aBuffNum != 2))
		{
		fprintf(stdout, "\n%s: WARNING! Valid buffer IDs are 1 or 2. Defaulting to 1.\n", __argv[0]);
		buff_num = 1;
		}

	fp_packet_r503 finger_pckt = { 0, };
	uint8_t data[2] = { R503_INSTR_IMG_TO_CHAR_FILE, buff_num };
	int Err = 0;
	//TODO: create one function which uses ctor, send and response commands
	if ((Err = CtorFpPacket(&finger_pckt, R503_PACKET_CMD, 0x4, data, aSerHandle)))
		{
		fprintf(stderr, "\n%s: ERROR! Could not create packet for finger IMG to Buffer conversion.\n", __argv[0]);
		return Err;
		}

	if ((Err = SendFpPacket(&finger_pckt)))
		{
		DtorFpPacket(&finger_pckt);
		return Err;
		}

	if ((Err = GetFpResponse()))
		{
		fprintf(stderr, "\n%s: ERROR! Failed to generate character file from finger IMG.\n", __argv[0]);
		DtorFpPacket(&finger_pckt);
		return Err;
		}

	DtorFpPacket(&finger_pckt);
	return EOk;
	}

int GenFingerTemplate(const int * const aSerHandle, const int * const aKillSig)
	{
	if (!aSerHandle)
		return ENullPtr;

	int kill = FALSE, Err = 0;
	fprintf(stdout, "\n%s: Creating finger template ...\n", __argv[0]);
	if (aKillSig)
		{
		if ((Err = GetFingerImg(aSerHandle, aKillSig)))
			return Err;
		kill = *aKillSig;
		}
	else
		{
		if ((Err = GetFingerImg(aSerHandle, NULL)))
			return Err;
		}

	if (!kill)
		{
		FingerImgToBuffer(aSerHandle, 1);

		if (aKillSig)
			{
			if ((Err = GetFingerImg(aSerHandle, aKillSig)))
				return Err;
			kill = *aKillSig;
			}
		else
			{
			if ((Err = GetFingerImg(aSerHandle, NULL)))
				return Err;
			}
		if (!kill)
			FingerImgToBuffer(aSerHandle, 2);
		}

	if (!kill)
		{
		fp_packet_r503 template_pckt = { 0, };
		uint8_t data[1] = { R503_INSTR_GEN_TEMPLATE };
		if ((Err = CtorFpPacket(&template_pckt, R503_PACKET_CMD, 0x3, data, aSerHandle)))
			{
			fprintf(stderr, "\n%s: ERROR! Could not create packet for finger template creation.\n", __argv[0]);
			return Err;
			}

		if ((Err = SendFpPacket(&template_pckt)))
			{
			DtorFpPacket(&template_pckt);
			return Err;
			}

		Err = GetFpResponse();
		if (Err == R503_ACK_ERR_COMBINE_FILES)
			{
			fprintf(stderr, "\n%s: ERROR! Fingers do not match.\n", __argv[0]);
			DtorFpPacket(&template_pckt);
			return Err;
			}
		else if (Err)
			{
			fprintf(stderr, "\n%s: ERROR! Failed to generate finger template.\n", __argv[0]);
			DtorFpPacket(&template_pckt);
			return Err;
			}

		fprintf(stdout, "\n%s: Finger template created.\n", __argv[0]);
		DtorFpPacket(&template_pckt);
		}

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
	int Err = 0;
	if ((Err = CtorFpPacket(&led_pckt, R503_PACKET_CMD, 0x7, data, aSerHandle)))
		{
		fprintf(stderr, "\n%s: ERROR! Could not create packet for LED settings.\n", __argv[0]);
		return Err;
		}

	if ((Err = SendFpPacket(&led_pckt)))
		{
		DtorFpPacket(&led_pckt);
		return Err;
		}

	if ((Err = GetFpResponse()))
		{
		DtorFpPacket(&led_pckt);
		return Err;
		}

	return EOk;
	}

int WaitForData(const int aSerHandle)
	{
	uint32_t start_t = gpioTick(), curr_t = 0;
	while (serDataAvailable(aSerHandle) == 0)
		{
		curr_t = gpioTick();
		if ((curr_t - start_t) > DATA_WAIT_TIMEOUT_MICROS)
			{
			fprintf(stderr, "\n%s: ERROR! Waiting for serial data reached timeout.\n", __argv[0]);
			return ETtyTimeout;
			}
		}

	return EOk;
	}

int64_t ReadByBytes(const int aSerHandle, unsigned aNumofBytes)
	{
	if (aNumofBytes > 4)
		{
		fprintf(stderr, "\n%s: WARNING! Largest packet element is 4 bytes. Larger values defaulted to 4.\n", __argv[0]);
		aNumofBytes = 4;
		}
	if (aNumofBytes < 1)
		{
		fprintf(stderr, "\n%s: WARNING! Smallest packet element is 1 byte. Smaller values defaulted to 1.\n", __argv[0]);
		aNumofBytes = 1;
		}

	int Err = 0;
	uint32_t read_value = 0;
	if ((Err = WaitForData(aSerHandle)))
		return Err;

	read_value = serReadByte(aSerHandle);
	for (size_t n_byte = 2; n_byte <= aNumofBytes; ++n_byte)
		{
		read_value <<= 8;
		if ((Err = WaitForData(aSerHandle))) return Err;
		read_value |= serReadByte(aSerHandle);
		}

	return (int64_t)read_value;
	}

int ReadFpPacket(fp_packet_r503 * const aPacket)
	{
	if (!aPacket)
		return ENullPtr;

	if (serDataAvailable(aPacket->serial_handle) == PI_BAD_HANDLE)
		{
		DtorFpPacket(aPacket);
		return ETtyBadHandle;
		}

	int64_t out = ReadByBytes(aPacket->serial_handle, 2);
	if (out < 0)
		{
		DtorFpPacket(aPacket);
		return out;
		}
	aPacket->header = (uint16_t)out;

	out = ReadByBytes(aPacket->serial_handle, 4);
	if (out < 0)
		{
		DtorFpPacket(aPacket);
		return out;
		}
	aPacket->address = (uint32_t)out;

	out = ReadByBytes(aPacket->serial_handle, 1);
	if (out < 0)
		{
		DtorFpPacket(aPacket);
		return out;
		}
	aPacket->package_id = (uint8_t)out;

	out = ReadByBytes(aPacket->serial_handle, 2);
	if (out < 0)
		{
		DtorFpPacket(aPacket);
		return out;
		}
	aPacket->package_length = (uint16_t)out;

	if (BufferAlloc(&aPacket->data, aPacket->package_length - sizeof(aPacket->checksum)))
		{
		DtorFpPacket(aPacket);
		return EBadAlloc;
		}
	for (size_t data_byte = 0; data_byte < aPacket->package_length - 2; ++data_byte)
		{
		out = ReadByBytes(aPacket->serial_handle, 1);
		if (out < 0)
			{
			DtorFpPacket(aPacket);
			return out;
			}
		aPacket->data[data_byte] = (uint8_t)out;
		}

	out = ReadByBytes(aPacket->serial_handle, 2);
	if (out < 0)
		{
		DtorFpPacket(aPacket);
		return out;
		}
	aPacket->checksum = (uint16_t)out;

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
	int Err = 0;
	fp_packet_r503 ack_pckt = { 0, };
	if ((Err = ReadFpPacket(&ack_pckt)))
		return Err;
	int out = ack_pckt.data[0];
	DtorFpPacket(&ack_pckt);

	return out;
	}
