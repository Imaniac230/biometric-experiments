#!/bin/bash

#sudo ./sync_ntp.sh

printf "\nStarting Raspberry Pi OS update ...\n\n"

sudo apt-get update

sleep 1

sudo apt-get upgrade

printf "\nOS update finished ...\n\n"

./updatepigpio.sh

#./updateptp4l.sh

#./check_rpi_config.sh
