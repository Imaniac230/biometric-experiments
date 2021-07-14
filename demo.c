#include "r503_fingerprint.h"


int main(int argc, char ** argv)
	{
	__argc = argc;
	__argv = argv;

	int Err = 0, terminate = FALSE;
	unsigned serhandle = 0;
	uint8_t flash_mem[2] = { 0x00, 0x00 };

	if(GpioConfig(NULL, &terminate))
		return EGpioBadInit;

	if ((Err = serOpen(UART_PORT_NAME, UART_BAUD_RATE, 0)) < 0)
		{
		fprintf(stderr, "\n%s: ERROR! Could not open serial port %s (handle %d).\n", __argv[0], UART_PORT_NAME, serhandle);
		return ETtyBadOpen;
		}
	serhandle = Err;

	SetFpLed(serhandle, R503_LED_FLASHING, R503_LED_PURPLE, 0x20, 0x10);

	fprintf(stdout, "\n%s: Generate and store a reference finger template:", argv[0]);
	if ((Err = GenFingerTemplate(serhandle, &terminate, flash_mem)))
		{
		if (Err != ESigKill)
			fprintf(stdout, "\n%s: Template creation unsuccessful.\n", argv[0]);
		SetFpLed(serhandle, R503_LED_OFF, 0x00, 0x00, 0x00);
		serClose(serhandle);
		return EOk;
		}


	fprintf(stdout, "\n\n\n%s: Match a finger with the stored template:", argv[0]);
	if ((Err = GenFingerTemplate(serhandle, &terminate, NULL)))
		{
		if (Err != ESigKill)
			fprintf(stdout, "\n%s: Template creation unsuccessful.\n", argv[0]);
		SetFpLed(serhandle, R503_LED_OFF, 0x00, 0x00, 0x00);
		serClose(serhandle);
		return EOk;
		}
	if ((Err = LoadFingerTemplate(serhandle, flash_mem, 2)))
		{
		fprintf(stdout, "\n%s: Stored template loading unsuccessful.\n", argv[0]);
		SetFpLed(serhandle, R503_LED_OFF, 0x00, 0x00, 0x00);
		serClose(serhandle);
		return EOk;
		}

	Err = MatchFingerTemplates(serhandle);
	if (!Err)
		{
		fprintf(stdout, "\n\n%s: SUCCESS! Finger matches.\n", argv[0]);
		SetFpLed(serhandle, R503_LED_FLASHING, R503_LED_BLUE, 0x20, 0x05);
		gpioDelay((uint32_t)1500000);
		SetFpLed(serhandle, R503_LED_SLOW_ON, R503_LED_BLUE, 0x20, 0x00);
		}
	else if (Err == R503_ACK_ERR_NO_FP_MATCH)
		{
		fprintf(stdout, "\n\n%s: FAIL! Finger doesn't match.\n", argv[0]);
		SetFpLed(serhandle, R503_LED_FLASHING, R503_LED_RED, 0x20, 0x05);
		gpioDelay((uint32_t)1500000);
		SetFpLed(serhandle, R503_LED_SLOW_ON, R503_LED_RED, 0x20, 0x00);
		}
	else
		{
		fprintf(stdout, "\n%s: Template matching unsuccessful.\n", argv[0]);
		SetFpLed(serhandle, R503_LED_OFF, 0x00, 0x00, 0x00);
		serClose(serhandle);
		return EOk;
		}


	while (!terminate)
		{}

	SetFpLed(serhandle, R503_LED_OFF, 0x00, 0x00, 0x00);
	serClose(serhandle);
	return EOk;
	}
