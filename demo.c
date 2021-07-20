#include "r503_fingerprint.h"


int main(int argc, char ** argv)
	{
	__argc = argc;
	__argv = argv;

	int Err = 0, terminate = FALSE;
	uint8_t flash_mem[2] = { 0x00, 0x00 };
#ifdef PIGPIO_PERMITTED
	if(GpioConfig(NULL, &terminate))
		return EGpioBadInit;
#endif

	fprintf(stdout, "\n%s: Generate and store a reference finger template:", argv[0]);
	if ((Err = GenFingerTemplate(flash_mem, &terminate)))
		{
		if (Err != ESigKill)
			fprintf(stdout, "\n%s: Template creation unsuccessful.\n", argv[0]);
		SetFpLed(R503_LED_OFF, 0x00, 0x00, 0x00);
		return EOk;
		}


	fprintf(stdout, "\n\n\n%s: Match a finger with the stored template:", argv[0]);
	if ((Err = GenFingerTemplate(NULL, &terminate)))
		{
		if (Err != ESigKill)
			fprintf(stdout, "\n%s: Template creation unsuccessful.\n", argv[0]);
		SetFpLed(R503_LED_OFF, 0x00, 0x00, 0x00);
		return EOk;
		}
	if ((Err = LoadFingerTemplate(flash_mem, 2)))
		{
		fprintf(stdout, "\n%s: Stored template loading unsuccessful.\n", argv[0]);
		SetFpLed(R503_LED_OFF, 0x00, 0x00, 0x00);
		return EOk;
		}

	Err = MatchFingerTemplates();
	if (!Err)
		{
		fprintf(stdout, "\n\n%s: SUCCESS! Finger matches.\n", argv[0]);
		SetFpLed(R503_LED_FLASHING, R503_LED_BLUE, 0x20, 0x05);
		WaitMicros((uint32_t)1500000);
		SetFpLed(R503_LED_SLOW_ON, R503_LED_BLUE, 0x20, 0x00);
		}
	else if (Err == R503_ACK_ERR_NO_FP_MATCH)
		{
		fprintf(stdout, "\n\n%s: FAIL! Finger doesn't match.\n", argv[0]);
		SetFpLed(R503_LED_FLASHING, R503_LED_RED, 0x20, 0x05);
		WaitMicros((uint32_t)1500000);
		SetFpLed(R503_LED_SLOW_ON, R503_LED_RED, 0x20, 0x00);
		}
	else
		{
		fprintf(stdout, "\n%s: Template matching unsuccessful.\n", argv[0]);
		SetFpLed(R503_LED_OFF, 0x00, 0x00, 0x00);
		return EOk;
		}

	while (!terminate)
		{}

	SetFpLed(R503_LED_OFF, 0x00, 0x00, 0x00);
#ifdef PIGPIO_PERMITTED
	GpioCleanup();
#endif
	return EOk;
	}
