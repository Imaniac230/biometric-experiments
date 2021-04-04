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

void BufferDeAlloc(uint8_t ** aBuffer)
	{
	if (aBuffer)
		{
		free(*aBuffer);
		*aBuffer = NULL;
		}
	}

int CtorFpPacket(fp_packet_r503 * const aStruct, const uint8_t aId, const uint16_t aLen, uint8_t * aData)
	{
	if (!aStruct)
		return ENullPtr;

	aStruct->header = FINGERPRINT_MSG_HEADER;
	aStruct->address = 0xFFFFFFFF;
	aStruct->pckg_id = aId;
	aStruct->pckg_len = aLen;

	if (BufferAlloc(&aStruct->data, aLen - sizeof(aStruct->chcksum)))
		return EBadAlloc;
	memcpy(aStruct->data, aData, aLen - sizeof(aStruct->chcksum));

	//sum -> over each single byte
	//should work, if not try summing whole 16bit length instead of separating to 2 8bit values
	aStruct->chcksum = aStruct->pckg_id + (uint8_t)(aStruct->pckg_len >> 8) + (uint8_t)(aStruct->pckg_len & 0xFF);
	for (size_t data_byte = 0; data_byte < aStruct->pckg_len - 2; ++data_byte)
		aStruct->chcksum += aStruct->data[data_byte];

	aStruct->full_pckt_len = sizeof(aStruct->header) + sizeof(aStruct->address) + sizeof(aStruct->pckg_id) + sizeof(aStruct->pckg_len) + aStruct->pckg_len;
	if (BufferAlloc(&aStruct->full_pckt, aStruct->full_pckt_len))
		return EBadAlloc;
	aStruct->full_pckt[0] = aStruct->header >> 8;
	aStruct->full_pckt[1] = aStruct->header & 0xFF;
	aStruct->full_pckt[2] = aStruct->address >> 24;
	aStruct->full_pckt[3] = (aStruct->address >> 16) & 0xFF;
	aStruct->full_pckt[4] = (aStruct->address >> 8) & 0xFF;
	aStruct->full_pckt[5] = aStruct->address & 0xFF;
	aStruct->full_pckt[6] = aStruct->pckg_id;
	aStruct->full_pckt[7] = aStruct->pckg_len >> 8;
	aStruct->full_pckt[8] = aStruct->pckg_len & 0xFF;
	for (size_t data_byte = 0; data_byte < aStruct->pckg_len; ++data_byte)
		aStruct->full_pckt[9 + data_byte] = aStruct->data[data_byte];
	aStruct->full_pckt[aStruct->full_pckt_len - 2] = aStruct->chcksum >> 8;
	aStruct->full_pckt[aStruct->full_pckt_len - 1] = aStruct->chcksum & 0xFF;

	return EOk;
	}

/*int CtorTimelb(timelb * const aStruct, const size_t aLen)
	{
	if(!aStruct)
		return ENullPtr;

	if(BufferAlloc(&aStruct->buffer_s, aLen))
		return EBadAlloc;
	if(BufferAlloc(&aStruct->buffer_ns, aLen))
		return EBadAlloc;
	if(BufferAlloc(&aStruct->buffer_isrticks, aLen))
		return EBadAlloc;

	aStruct->buffer_len = aLen;
	aStruct->val_idx = 0;

	return EOk;
	}

int DtorTimelb(timelb * aStruct)
	{
	if(!aStruct)
		return ENullPtr;

	BufferDeAlloc(&aStruct->buffer_s);
	BufferDeAlloc(&aStruct->buffer_ns);
	BufferDeAlloc(&aStruct->buffer_isrticks);
	aStruct->buffer_len = 0;
	aStruct->val_idx = 0;

	return EOk;
	}*/
