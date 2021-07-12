#include "r503_fingerprint.h"


const int16_t * GetFingerprintData()
	{
	char **argv = (char**)malloc(sizeof(char[1][20]));
	argv[0] = "GetFingerprintData";
	__argv = argv;
	int terminate = FALSE, serhandle = 0;
	static int16_t Err[1] = { 0 };

	if(GpioConfig(NULL, &terminate))
		{ free(argv); Err[0] = EGpioBadInit; return Err; }

	if ((serhandle = serOpen(UART_PORT_NAME, UART_BAUD_RATE, 0)) < 0)
		{
		fprintf(stderr, "\n%s: ERROR! Could not open serial port %s (handle %d).\n", argv[0], UART_PORT_NAME, serhandle);
		free(argv); Err[0] = ETtyBadOpen; return Err;
		}

	if ((Err[0] = GenFingerTemplate(&serhandle, &terminate, NULL)))
		{
		fprintf(stderr, "\n%s: ERROR! Could not generate finger template.\n", argv[0]);
		free(argv); return Err;
		}

	const int16_t *FingerTemplate = NULL;
	FingerTemplate = ExportFingerTemplate(&serhandle, 1, NULL/*"finger_template.fpt"*/);
	if (FingerTemplate[0] < 0)
		{
		fprintf(stderr, "\n%s: ERROR! Could not export finger template.\n", argv[0]);
		free(argv); return FingerTemplate;
		}
/*
	size_t idx = 0;
	printf("\n\ttemplate (in C func):\n");
	while (idx < R503_MAX_PACKET_DATA_LENGTH + 1)
		{
		printf("%d", FingerTemplate[idx]);
		++idx;
		}
	printf("%d\n", FingerTemplate[idx]);
*/
	serClose(serhandle);
	free(argv);
	return FingerTemplate;
	}
