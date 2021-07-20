# biometric-experiments

experimenting with R503 capacitive fingerprint sensor.

# Usage
This code is written to work with the R503 capacitive fingerprint sensor. A Raspberry Pi 3B+ was used for main development. Communication is acheived through a serial port. Make sure the `UART_BAUD_RATE` parameter in `r503_fingerprint.h` is corresponding to the actual sensor configuration (57600 is the default sensor setting).

Run a simple demo with `sudo ./rundemo.sh`.

To make a shared external library usable in Python run `./makesharedlib.sh`.
## Raspberry Pi
The code works best with the [PiGpio](http://abyz.me.uk/rpi/pigpio/) library. Install the latest version with `./updatepigpio.sh`.

The full UART (`/dev/ttyAMA0`) is used for communication for best results. Add `dtoverlay=disable-bt` or `miniuart-bt` to `/boot/config.txt` to make it the primary UART.
If Bluetooth is disabled `sudo systemctl disable hciuart` may need to be run as well.
Also see `check_config.sh`.

With PiGpio, the program must be run with root priviledges.
## Other linux
The code should also work on other non-Raspberry systems with linux, but proper function is not guaranteed.

Make sure the `UART_PORT_NAME` parameter in `r503_fingerprint.h` is set to the correct port to be used.
