#include "r503_fingerprint.h"

int __argc = 0;
char ** __argv = NULL;


void HandleFinger(int aGpio, int aLevel, uint32_t aTick, void * aData)
	{
	fprintf(stdout, "finger pressed\n");
	}

void HandleSignal(int aSignum, void * aData)
	{
	*(int*)aData = TRUE;
	}


int SetArgv(char * const aStr, const int aFree)
	{
	if (!__argv && aStr)
		{
		__argv = (char**)malloc(sizeof(char[1][100]));
		__argv[0] = aStr; __argc = -1;
		return TRUE;
		}

	if ((__argc == -1) && aFree)
		{
		free(__argv); __argv = NULL;
		return FALSE;
		}

	return FALSE;
	}

int MaxPacketDataLen()
	{ return R503_MAX_PACKET_DATA_LENGTH; }

uint32_t GetMicroTick()
	{
	return gpioTick();
	}

void WaitMicros(const uint32_t aValue)
	{
	gpioDelay(aValue);
	}

int OpenFpSerialPort()
	{
	int is_allocated = SetArgv("OpenFpSerialPort", FALSE), Err = 0;
	if ((Err = serOpen(UART_PORT_NAME, UART_BAUD_RATE, 0)) < 0)
		{
		fprintf(stderr, "\n%s: ERROR! Could not open serial port %s. (code %d)\n", __argv[0], UART_PORT_NAME, Err);
		SetArgv(NULL, is_allocated); return ETtyBadOpen;
		}

	SetArgv(NULL, is_allocated);
	return Err;
	}

int CloseFpSerialPort(const unsigned aSerHandle)
	{
	int is_allocated = SetArgv("CloseFpSerialPort", FALSE), Err = serClose(aSerHandle);
	if (Err)
		fprintf(stdout, "\n%s: WARNING! Could not close serial port with handle %u, was not assigned to any open port. (code %d)", __argv[0], aSerHandle, Err);

	SetArgv(NULL, is_allocated);
	return Err;
	}

int DataOnFpSerial(const unsigned aSerHandle)
	{
	int is_allocated = SetArgv("DataOnFpSerial", FALSE), Bytes = serDataAvailable(aSerHandle);
	if (Bytes == PI_BAD_HANDLE)
		{
		fprintf(stderr, "\n%s: ERROR! Bad serial handle. (code %d)\n", __argv[0], ETtyBadHandle);
		SetArgv(NULL, is_allocated); return ETtyBadHandle;
		}

	SetArgv(NULL, is_allocated);
	return Bytes;
	}

int GpioConfig(int *aIsrData, int *aSigData)
	{
	int is_allocated = SetArgv("GpioConfig", FALSE);

	if (gpioInitialise() < 0)
		{
		fprintf(stderr, "\n%s: ERROR! Could not initialise pigpio library. (code %d)\n", __argv[0], EGpioBadInit);
		SetArgv(NULL, is_allocated); return EGpioBadInit;
		}

	if (gpioSetMode(GPIO_FINGER_WAKEUP, PI_INPUT) == PI_BAD_GPIO)
		{
		fprintf(stderr, "\n%s: ERROR! Could not set input mode on GPIO %d. (code %d)\n", __argv[0], GPIO_FINGER_WAKEUP, EGpioBadMode);
		SetArgv(NULL, is_allocated); return EGpioBadMode;
		}

	if (gpioSetPullUpDown(GPIO_FINGER_WAKEUP, PI_PUD_OFF))
		{
		fprintf(stderr, "\n%s: ERROR! Could not set off pull up/down on GPIO %d. (code %d)\n", __argv[0], GPIO_FINGER_WAKEUP, EGpioBadPud);
		SetArgv(NULL, is_allocated); return EGpioBadPud;
		}

	if (aSigData)
		{
		if (gpioSetSignalFuncEx(SIGNAL_TERMINATE, HandleSignal, (void*)aSigData))
			{
			fprintf(stderr, "\n%s: ERROR! Could not set signal function on signal %d. (code %d)\n", __argv[0], SIGNAL_TERMINATE, EGpioBadISR);
			SetArgv(NULL, is_allocated); return EGpioBadISR;
			}
		}

	if (aIsrData)
		{
		if(gpioSetISRFuncEx(GPIO_FINGER_WAKEUP, ISR_DETECTION_LEVEL, 0, HandleFinger, (void*)aIsrData))
			{
			fprintf(stderr, "\n%s: ERROR! Could not set ISR function on GPIO %d. (code %d)\n", __argv[0], GPIO_FINGER_WAKEUP, EGpioBadISR);
			SetArgv(NULL, is_allocated); return EGpioBadISR;
			}
		}

	SetArgv(NULL, is_allocated);
	return EOk;
	}

int GpioCleanup()
	{
	int is_allocated = SetArgv("GpioCleanup", FALSE);

	if (gpioSetSignalFuncEx(SIGNAL_TERMINATE, 0, 0))
		{
		fprintf(stderr, "\n%s: ERROR! Could not turn off signal handling on signal %d. (code %d)\n", __argv[0], SIGNAL_TERMINATE, EGpioBadISR);
		SetArgv(NULL, is_allocated); return EGpioBadISR;
		}

	if(gpioSetISRFuncEx(GPIO_FINGER_WAKEUP, ISR_DETECTION_LEVEL, 0, 0, 0))
		{
		fprintf(stderr, "\n%s: ERROR! Could not turn off ISR handling on GPIO %d. (code %d)\n", __argv[0], GPIO_FINGER_WAKEUP, EGpioBadISR);
		SetArgv(NULL, is_allocated); return EGpioBadISR;
		}

	gpioTerminate();
	SetArgv(NULL, is_allocated);
	return EOk;
	}

int BufferAlloc(uint8_t ** const aBuffer, const size_t aLen)
	{
	if (!aBuffer)
		return ENullPtr;

	if (*aBuffer)
		BufferDealloc(aBuffer);

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

int CtorFpPacket(fp_packet_r503 * const aStruct, const uint8_t aId, const uint16_t aLen, const uint8_t * const aData, const unsigned aSerHandle)
	{
	if (!aStruct || !aData)
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

	aStruct->serial_handle = aSerHandle;

	return EOk;
	}

void DtorFpPacket(fp_packet_r503 * aStruct)
	{
	if (aStruct)
		{
		aStruct->header = 0;
		aStruct->address = 0;
		aStruct->package_id = 0;
		aStruct->package_length = 0;
		aStruct->checksum = 0;
		aStruct->send_packet_length = 0;
		aStruct->serial_handle = 0;

		BufferDealloc(&aStruct->data);
	        BufferDealloc(&aStruct->send_packet);
		}
	}

int GetFingerImg(const int * const aKillSig)
	{
	int Err = 0;
	if ((Err = OpenFpSerialPort()) < 0)
		return ETtyBadOpen;
	unsigned serhandle = (unsigned)Err;

	int is_allocated = SetArgv("GetFingerImg", FALSE);
	fp_packet_r503 finger_pckt = { 0, };
	uint8_t data[1] = { R503_INSTR_GET_FINGER_IMG };
	if ((Err = CtorFpPacket(&finger_pckt, R503_PACKET_CMD, 0x3, data, serhandle)))
		{
		fprintf(stderr, "\n%s: ERROR! Could not create packet for finger IMG storage command. (code %d)\n", __argv[0], Err);
		CloseFpSerialPort(serhandle); SetArgv(NULL, is_allocated);
		return Err;
		}

	fprintf(stdout, "\n%s: \tPlace finger on sensor for detection ...\n", __argv[0]);
	SetFpLed(R503_LED_FLASHING, R503_LED_PURPLE, 0x20, R503_LED_CYCLES_INFINITE);
	Err = R503_ACK_ERR_NO_FINGER;
	while (Err != R503_ACK_OK)
		{
		if ((Err = SendFpPacket(&finger_pckt)))
			{
			SetFpLed(R503_LED_SLOW_OFF, R503_LED_RED, R503_LED_PERIOD_MAX, 0);
			DtorFpPacket(&finger_pckt); CloseFpSerialPort(serhandle);
			SetArgv(NULL, is_allocated); return Err;
			}

		Err = GetFpResponse(serhandle);
		if ((Err != R503_ACK_OK) && (Err != R503_ACK_ERR_NO_FINGER))
			{
			fprintf(stderr, "\n%s: ERROR! Error when detecting finger. (code %d)\n", __argv[0], Err);
			SetFpLed(R503_LED_SLOW_OFF, R503_LED_RED, R503_LED_PERIOD_MAX, 0);
			DtorFpPacket(&finger_pckt); CloseFpSerialPort(serhandle);
			SetArgv(NULL, is_allocated); return Err;
			}

		if (aKillSig && *aKillSig)
			{
			SetFpLed(R503_LED_OFF, 0, 0, 0);
			DtorFpPacket(&finger_pckt); CloseFpSerialPort(serhandle);
			SetArgv(NULL, is_allocated); return ESigKill;
			}
		}
	fprintf(stdout, "\n%s: \tFinger image stored successfully.\n\n", __argv[0]);
	SetFpLed(R503_LED_SLOW_OFF, R503_LED_BLUE, R503_LED_PERIOD_MAX, 0);

	DtorFpPacket(&finger_pckt);
	CloseFpSerialPort(serhandle);
	SetArgv(NULL, is_allocated);
	return EOk;
	}

int SendFpWithResponse(const uint8_t aId, const uint16_t aLen, const uint8_t * const aData, const unsigned * const aSerHandle, const char * const aErrStr)
	{
	if (!aData)
		return ENullPtr;

	int Err = 0;
	unsigned serhandle = 0;
	if (aSerHandle)
		serhandle = *aSerHandle;
	else
		{
		if ((Err = OpenFpSerialPort()) < 0)
			return ETtyBadOpen;
		serhandle = (unsigned)Err;
		}

	int is_allocated = SetArgv("SendFpWithResponse", FALSE);
	fp_packet_r503 packet = { 0, };
	if ((Err = CtorFpPacket(&packet, aId, aLen, aData, serhandle)))
		{
		if (aErrStr)
			fprintf(stderr, "\n%s: ERROR! Could not create packet %s (code %d)\n", __argv[0], aErrStr, Err);
		else
			fprintf(stderr, "\n%s: ERROR! Could not create packet. (code %d)\n", __argv[0], Err);
		if (!aSerHandle)
			CloseFpSerialPort(serhandle);
		DtorFpPacket(&packet);
		SetArgv(NULL, is_allocated);
		return Err;
		}

	if ((Err = SendFpPacket(&packet)))
		{
		if (!aSerHandle)
			CloseFpSerialPort(serhandle);
		DtorFpPacket(&packet);
		SetArgv(NULL, is_allocated);
		return Err;
		}

	Err = GetFpResponse(serhandle);
	if (!aSerHandle)
		CloseFpSerialPort(serhandle);
	DtorFpPacket(&packet);
	SetArgv(NULL, is_allocated);

	return Err;
	}

int FingerImgToBuffer(const int aBuffNum)
	{
	int is_allocated = SetArgv("FingerImgToBuffer", FALSE);
	uint8_t buff_num = (uint8_t)aBuffNum;
	if ((aBuffNum != 1) && (aBuffNum != 2))
		{
		fprintf(stdout, "\n%s: WARNING! Valid buffer IDs are 1 or 2. Defaulting to 1.\n", __argv[0]);
		buff_num = 1;
		}

	uint8_t data[2] = { R503_INSTR_IMG_TO_CHAR_FILE, buff_num };
	int Err = SendFpWithResponse(R503_PACKET_CMD, 0x4, data, NULL, "for finger IMG to Buffer conversion.");
	if (Err)
		{
		fprintf(stderr, "\n%s: ERROR! Failed to generate character file from finger IMG. (code %d)\n", __argv[0], Err);
		SetArgv(NULL, is_allocated); return Err;
		}

	SetArgv(NULL, is_allocated);
	return EOk;
	}

int SaveFingerTemplate(const int aSrcBuffNum, const uint8_t * const aDstFlashPos)
	{
	int is_allocated = SetArgv("SaveFingerTemplate", FALSE);
	if (!aDstFlashPos)
		{
		fprintf(stderr, "\n%s: ERROR! Memory storage location not specified. (code %d)\n", __argv[0], ENullPtr);
		SetArgv(NULL, is_allocated); return ENullPtr;
		}

	uint8_t buff_num = (uint8_t)aSrcBuffNum;
	if ((aSrcBuffNum != 1) && (aSrcBuffNum != 2))
		{
		fprintf(stdout, "\n%s: WARNING! Valid buffer IDs are 1 or 2. Defaulting to 1.\n", __argv[0]);
		buff_num = 1;
		}

	fprintf(stdout, "\n\n%s: \tSaving finger template to flash memory ...\n", __argv[0]);
	uint8_t data[4] = { R503_INSTR_STORE_TEMPLATE, buff_num,  aDstFlashPos[0], aDstFlashPos[1] };
	int Err = SendFpWithResponse(R503_PACKET_CMD, 0x6, data, NULL, "for finger template storage.");
	if (Err == R503_ACK_ERR_FLASH_ID_OVERFLOW)
		fprintf(stderr, "\n%s: ERROR! Storage position out of bounds of flash memory. (code %d)", __argv[0], Err);
	if (Err)
		{
		fprintf(stderr, "\n%s: ERROR! Failed to save finger template to flash. (code %d)\n", __argv[0], Err);
		SetArgv(NULL, is_allocated); return Err;
		}
	fprintf(stdout, "\n%s: \tTemplate saved successfuly.\n", __argv[0]);
	SetArgv(NULL, is_allocated);

	return EOk;
	}

int LoadFingerTemplate(const uint8_t * const aSrcFlashPos, const int aDstBuffNum)
	{
	int is_allocated = SetArgv("LoadFingerTemplate", FALSE);
	if (!aSrcFlashPos)
		{
		fprintf(stderr, "\n%s: ERROR! Memory storage location not specified. (code %d)\n", __argv[0], ENullPtr);
		SetArgv(NULL, is_allocated); return ENullPtr;
		}

	uint8_t buff_num = (uint8_t)aDstBuffNum;
	if ((aDstBuffNum != 1) && (aDstBuffNum != 2))
		{
		fprintf(stdout, "\n%s: WARNING! Valid buffer IDs are 1 or 2. Defaulting to 1.\n", __argv[0]);
		buff_num = 1;
		}

	fprintf(stdout, "\n\n%s: \tLoading finger template from flash memory ...\n", __argv[0]);
	uint8_t data[4] = { R503_INSTR_READ_TEMPLATE, buff_num,  aSrcFlashPos[0], aSrcFlashPos[1] };
	int Err = SendFpWithResponse(R503_PACKET_CMD, 0x6, data, NULL, "for finger template loading.");
	if (Err == R503_ACK_ERR_FLASH_ID_OVERFLOW)
		fprintf(stderr, "\n%s: ERROR! Reading position out of bounds of flash memory. (code %d)", __argv[0], Err);
	if (Err)
		{
		fprintf(stderr, "\n%s: ERROR! Failed to load finger template from flash. (code %d)\n", __argv[0], Err);
		SetArgv(NULL, is_allocated); return Err;
		}
	fprintf(stdout, "\n%s: \tTemplate saved successfuly.\n", __argv[0]);
	SetArgv(NULL, is_allocated);

	return EOk;
	}

int GenFingerTemplate(const uint8_t * const aDstFlashPos, const int * const aKillSig)
	{
	int is_allocated = SetArgv("GenFingerTemplate", FALSE);
	int Err = 0;
	fprintf(stdout, "\n\n%s: \tCreating finger template ...\n", __argv[0]);
	if (aKillSig)
		{
		if ((Err = GetFingerImg(aKillSig)))
			{ SetArgv(NULL, is_allocated); return Err; }
		}
	else
		{
		if ((Err = GetFingerImg(NULL)))
			{ SetArgv(NULL, is_allocated); return Err; }
		}
	FingerImgToBuffer(1);

	if (aKillSig)
		{
		if ((Err = GetFingerImg(aKillSig)))
			{ SetArgv(NULL, is_allocated); return Err; }
		}
	else
		{
		if ((Err = GetFingerImg(NULL)))
			{ SetArgv(NULL, is_allocated); return Err; }
		}
	FingerImgToBuffer(2);

	uint8_t data[1] = { R503_INSTR_GEN_TEMPLATE };
	Err = SendFpWithResponse(R503_PACKET_CMD, 0x3, data, NULL, "for finger template creation.");
	if (Err == R503_ACK_ERR_COMBINE_FILES)
		{
		fprintf(stderr, "\n%s: ERROR! Fingers do not match. (code %d)", __argv[0], EFpBadMatch);
		SetArgv(NULL, is_allocated); return EFpBadMatch;
		}
	if (Err)
		{
		fprintf(stderr, "\n%s: ERROR! Failed to generate finger template. (code %d)\n", __argv[0], Err);
		SetArgv(NULL, is_allocated); return Err;
		}

	fprintf(stdout, "\n%s: \tFinger template created.\n", __argv[0]);

	if (aDstFlashPos)
		if ((Err = SaveFingerTemplate(1, aDstFlashPos)))
			{ SetArgv(NULL, is_allocated); return Err; }

	SetArgv(NULL, is_allocated);
	return EOk;
	}

int MatchFingerTemplates()
	{
	int is_allocated = SetArgv("MatchFingerTemplates", FALSE);
	uint8_t data[1] = { R503_INSTR_MATCH_TEMPLATES };
	int Err = SendFpWithResponse(R503_PACKET_CMD, 0x3, data, NULL, "for template matching.");
	if (Err == R503_ACK_ERR_NO_FP_MATCH)
		{
		fprintf(stdout, "\n%s: \tFinger templates do not match.\n", __argv[0]);
		SetArgv(NULL, is_allocated); return Err;
		}
	if (Err)
		{
		fprintf(stderr, "\n%s: ERROR! Could not perform finger template matching. (code %d)\n", __argv[0], Err);
		SetArgv(NULL, is_allocated); return Err;
		}
	fprintf(stdout, "\n%s: \tFinger templates match.\n", __argv[0]);
	SetArgv(NULL, is_allocated);

	return EOk;
	}

int SendFpPacket(const fp_packet_r503 * const aPacket)
	{
	if (!aPacket)
		return ENullPtr;

	int is_allocated = SetArgv("SendFpPacket", FALSE);
	for (size_t packet_byte = 0; packet_byte < aPacket->send_packet_length; ++packet_byte)
		{
		if (serWriteByte(aPacket->serial_handle, aPacket->send_packet[packet_byte]))
			{
			fprintf(stderr, "\n%s: ERROR! Could not send %zuth byte to serial port %s. (code %d)\n", __argv[0], packet_byte, UART_PORT_NAME, ETtyBadWrite);
			SetArgv(NULL, is_allocated); return ETtyBadWrite;
			}
		}

	SetArgv(NULL, is_allocated);
	return EOk;
	}

int SetFpLed(const uint8_t aState, const uint8_t aColor, const uint8_t aPeriod, const uint8_t aCount)
	{
	uint8_t data[5] = { R503_INSTR_LED_CONFIG, aState, aPeriod, aColor, aCount };
	int Err = SendFpWithResponse(R503_PACKET_CMD, 0x7, data, NULL, "for LED settings.");
	if (Err)
		return Err;

	return EOk;
	}

int WaitForData(const unsigned aSerHandle)
	{
	int is_allocated = SetArgv("WaitForData", FALSE), bytes_avail = 0;
	uint32_t start_t = GetMicroTick(), curr_t = 0;
	while ((bytes_avail = DataOnFpSerial(aSerHandle)) == 0)
		{
		if ((curr_t = GetMicroTick()) < start_t)
			fprintf(stdout, "\n%s: WARNING! Incorrect timeout calculated.\n", __argv[0]);
		if ((curr_t - start_t) > DATA_WAIT_TIMEOUT_MICROS)
			{
			fprintf(stderr, "\n%s: ERROR! Waiting for serial data reached timeout. (code %d)\n", __argv[0], ETtyTimeout);
			SetArgv(NULL, is_allocated); return ETtyTimeout;
			}
		}

	if (bytes_avail < 0)
		{ SetArgv(NULL, is_allocated); return bytes_avail; }

	SetArgv(NULL, is_allocated);
	return EOk;
	}

int64_t ReadByBytes(const unsigned aSerHandle, unsigned aNumofBytes)
	{
	int is_allocated = SetArgv("ReadByBytes", FALSE);
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
		{ SetArgv(NULL, is_allocated); return Err; }

	read_value = serReadByte(aSerHandle);
	for (size_t n_byte = 2; n_byte <= aNumofBytes; ++n_byte)
		{
		read_value <<= 8;
		if ((Err = WaitForData(aSerHandle)))
			{ SetArgv(NULL, is_allocated); return Err; }
		read_value |= serReadByte(aSerHandle);
		}

	SetArgv(NULL, is_allocated);
	return (int64_t)read_value;
	}

int ReadFpPacket(fp_packet_r503 * const aPacket)
	{
	if (!aPacket)
		return ENullPtr;

	int Err = 0;
	if ((Err = DataOnFpSerial(aPacket->serial_handle)) < 0)
		{
		DtorFpPacket(aPacket);
		return Err;
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

	if ((Err = BufferAlloc(&aPacket->data, aPacket->package_length - sizeof(aPacket->checksum))))
		{
		DtorFpPacket(aPacket);
		return Err;
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
	if (aPacket)
		{
		int is_allocated = SetArgv("PrintFpPacket", FALSE);
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
		SetArgv(NULL, is_allocated);
		}
	}

int GetFpResponse(const unsigned aSerHandle)
	{
	int is_allocated = SetArgv("GetFpResponse", FALSE), Err = 0;
	fp_packet_r503 ack_pckt = { 0, };
	ack_pckt.serial_handle = aSerHandle;
	if ((Err = ReadFpPacket(&ack_pckt)))
		{
		fprintf(stderr, "\n%s: ERROR! Could not read received packet. (code %d)\n", __argv[0], Err);
		DtorFpPacket(&ack_pckt); return Err;
		}
	int Ret = (int)ack_pckt.data[0];
	DtorFpPacket(&ack_pckt);
	SetArgv(NULL, is_allocated);

	return Ret;
	}

const int16_t * ExportFingerTemplate(const uint8_t aBuffNum, const char * const aFileName)
	{
	static int16_t Err[1] = { 0 };
	if ((Err[0] = OpenFpSerialPort()) < 0)
		return Err;
	unsigned serhandle = (unsigned)Err[0];

	int is_allocated = SetArgv("ExportFingerTemplate", FALSE);
	uint8_t buff_num = aBuffNum;
	if ((aBuffNum != 1) && (aBuffNum != 2))
		{
		fprintf(stdout, "\n%s: WARNING! Valid buffer IDs are 1 or 2. Defaulting to 1.\n", __argv[0]);
		buff_num = 1;
		}

	uint8_t data[2] = { R503_INSTR_UPLOAD_TEMPLATE, buff_num };
	Err[0] = SendFpWithResponse(R503_PACKET_CMD, 0x4, data, &serhandle, "for finger template upload.");
	if (Err[0])
		{
		fprintf(stderr, "\n%s: ERROR! Failed to generate character file from finger IMG. (code %d)\n", __argv[0], Err[0]);
		CloseFpSerialPort(serhandle); SetArgv(NULL, is_allocated);
		return Err;
		}

	const int16_t *PacketData = ExportFpPacketData(serhandle, aFileName);
	WaitMicros((uint32_t)150000);
	CloseFpSerialPort(serhandle);
	SetArgv(NULL, is_allocated);
	return PacketData;
	}

int ReadPrintFpPacket()
	{
	int is_allocated = SetArgv("ReadPrintFpPacket", FALSE);
	fp_packet_r503 pckt = { 0, };
	int Err = 0;

	if ((Err = ReadFpPacket(&pckt)))
		{
		fprintf(stderr, "\n%s: ERROR! Could not read received packet. (code %d)\n", __argv[0], Err);
		DtorFpPacket(&pckt); SetArgv(NULL, is_allocated);
		return Err;
		}
	PrintFpPacket(&pckt, "Finger template");

	DtorFpPacket(&pckt);
	SetArgv(NULL, is_allocated);
	return EOk;
	}

const int16_t * ExportFpPacketData(const unsigned aSerHandle, const char * const aFileName)
	{
	int is_allocated = SetArgv("ExportFpPacketData", FALSE);
	static int16_t Err[1] = { 0 };
	fp_packet_r503 pckt = { 0, };
	pckt.serial_handle = aSerHandle;
	if ((Err[0] = ReadFpPacket(&pckt)))
		{
		fprintf(stderr, "\n%s: ERROR! Could not read received packet. (code %d)\n", __argv[0], Err[0]);
		DtorFpPacket(&pckt); SetArgv(NULL, is_allocated);
		return Err;
		}

	if (aFileName)
		{
		FILE *outfile = fopen(aFileName,"wb");
		for (size_t idx = 0; idx < pckt.package_length - 2; ++idx)
			fprintf(outfile, "%x", pckt.data[idx]);

		fclose(outfile);
		fprintf(stdout, "\n%s: \tPacket data exported to \"%s\".\n", __argv[0], aFileName);
		}

	size_t ret_length = pckt.package_length - 2, ret_idx = 0;
	static int16_t PacketData[R503_MAX_PACKET_DATA_LENGTH + 1] = { 0, };
	if ((pckt.package_length - 2) > R503_MAX_PACKET_DATA_LENGTH)
		{
		fprintf(stdout, "\n%s: WARNING! Packet data larger than storage buffer (R503_MAX_PACKET_DATA_LENGTH). Function output will be incomplete.\n", __argv[0]);
		ret_length = R503_MAX_PACKET_DATA_LENGTH;
		}

	for (ret_idx = 0; ret_idx < ret_length; ++ret_idx)
		PacketData[ret_idx] = pckt.data[ret_idx];
	PacketData[ret_idx] = -1;

	DtorFpPacket(&pckt);
	SetArgv(NULL, is_allocated);
	return PacketData;
	}

const int16_t * GetFingerprintData()
	{
	int is_allocated = SetArgv("GetFingerprintData", FALSE), terminate = FALSE;
	static int16_t Err[1] = { 0 };

	if(GpioConfig(NULL, &terminate))
		{ SetArgv(NULL, is_allocated); Err[0] = EGpioBadInit; return Err; }

	if ((Err[0] = GenFingerTemplate(NULL, &terminate)))
		{
		fprintf(stderr, "\n%s: ERROR! Could not generate finger template. (code %d)\n", __argv[0], Err[0]);
		SetArgv(NULL, is_allocated); return Err;
		}

	const int16_t *FingerTemplate = NULL;
	FingerTemplate = ExportFingerTemplate(1, NULL/*"finger_template.fpt"*/);
	if (FingerTemplate[0] < 0)
		{
		fprintf(stderr, "\n%s: ERROR! Could not export finger template. (code %d)\n", __argv[0], FingerTemplate[0]);
		SetArgv(NULL, is_allocated); return FingerTemplate;
		}
/*
	size_t idx = 0;
	printf("\n\ttemplate (in C func):\n");
	while (idx < R503_MAX_PACKET_DATA_LENGTH + 1)
		{
		printf("%x", FingerTemplate[idx]);
		++idx;
		}
	printf("%x\n", FingerTemplate[idx]);
*/
	SetArgv(NULL, is_allocated);
	return FingerTemplate;
	}
