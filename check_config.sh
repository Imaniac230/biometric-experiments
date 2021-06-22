#!/bin/bash

primary_uart_set=$(cat /boot/config.txt | grep -E "dtoverlay=disable-bt|dtoverlay=miniuart-bt")
primary_uart=$(ls -l /dev/serial0 | grep ttyAMA0)

hciuart_dead=$(systemctl status hciuart | grep dead)


problem=0

if ! [[ "$primary_uart_set" && "$primary_uart" ]]; then
	printf "\nProgram might not work! Primary UART is not ttyAMA0!"
	printf "\nYou can add 'dtoverlay=disable-bt' to '/boot/config.txt' to disable Bluetooth and make the fast UART primary.\n\n"
	problem=1
fi

if ! [[ "$hciuart_dead" ]]; then
        printf "\nHciuart is probably enabled! If Bluetooth was disabled also run 'sudo systemctl disable hciuart' to disable the service.\n\n"
	problem=1
fi


if [ "$problem" -eq 0 ]; then
	printf "\nRaspberry Pi configuration OK.\n\n"
fi

