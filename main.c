#include "r503_fingerprint.h"


//TODO: store fingerprint template in a better format than just plain text?!
int main(int argc, char ** argv)
	{
	__argc = argc;
	__argv = argv;

	int Err = 0, terminate = FALSE/*, isr_uselater = 0,*/;
	const int16_t *fp_template = NULL;

	if(GpioConfig(NULL, &terminate))
		return EGpioBadInit;

	if ((Err = GenFingerTemplate(NULL, &terminate)))
		{
		fprintf(stderr, "\n%s: ERROR! Could not generate finger template (code %d).\n", argv[0], Err);
		return Err;
		}
	fp_template = ExportFingerTemplate(1, "finger_template.fpt");
	if (fp_template[0] < 0)
		{
		fprintf(stderr, "\n%s: ERROR! Could not export finger template. (code %d)\n", argv[0], fp_template[0]);
		return fp_template[0];
		}

	SetFpLed(R503_LED_FLASHING, R503_LED_RED, 0x20, 0x10);
/*
	while (!terminate)
		{}
*/
	GpioCleanup();
	return EOk;
	}
