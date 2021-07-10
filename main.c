#include "r503_fingerprint.h"


//TODO: get rid of serhandle pointer types where not needed
//	add serhandle to print, get and their derivatives?
//	store fingerprint template in a better format than just plain text?!
int main(int argc, char ** argv)
	{
	__argc = argc;
	__argv = argv;

	int Err = 0, terminate = FALSE, /*isr_uselater = 0,*/ serhandle = 0;

	if(GpioConfig(NULL, &terminate))
		return EGpioBadInit;

	if ((serhandle = serOpen(UART_PORT_NAME, UART_BAUD_RATE, 0)) < 0)
		{
		fprintf(stderr, "\n%s: ERROR! Could not open serial port %s (handle %d).\n", argv[0], UART_PORT_NAME, serhandle);
		return ETtyBadOpen;
		}

	if ((Err = GenFingerTemplate(&serhandle, &terminate, NULL)))
		{
		fprintf(stderr, "\n%s: ERROR! Could not generate finger template.\n", argv[0]);
		return Err;
		}
	if ((Err = ExportFingerTemplate(&serhandle, 1, "finger_template.fpt")))
		{
		fprintf(stderr, "\n%s: ERROR! Could not export finger template.\n", argv[0]);
		return Err;
		}

	SetFpLed(&serhandle, R503_LED_FLASHING, R503_LED_PURPLE, 0x20, 0x10);

/*	while (!terminate)
		{}
*/
	serClose(serhandle);
	return EOk;
	}
