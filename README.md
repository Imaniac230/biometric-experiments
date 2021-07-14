# biometric-experiments

experimenting with R503 capacitive fingerprint sensor on a Raspbery Pi 3B+

# Usage

To use this code, the [PiGpio](http://abyz.me.uk/rpi/pigpio/) library is required. Install the latest version with `./updatepigpio.sh`.

The full UART (`/dev/ttyAMA0`) is used for communication for best results. Add `dtoverlay=disable-bt` or `miniuart-bt` to `/boot/config.txt` to make it the primary UART.
If Bluetooth is disabled `sudo systemctl disable hciuart` may need to be run as well.
Also see `check_config.sh`.

Make sure the `UART_BAUD_RATE` parameter in `r503_fingerprint.h` is corresponding to the actual sensor configuration (57600 is the default sensor setting).

Run a simple demo with `sudo ./rundemo.sh`.

To make a shared external library usable in Python run `./makesharedlib.sh`.
