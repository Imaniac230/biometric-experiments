#!/bin/bash

./check_rpi_config.sh

printf "\nStarting program ...\n\n"

gcc -Wall -Werror -pthread -o demo demo.c R503_fingerprint.c R503_fingerprint.h -lpigpio -lrt && ./demo && rm demo

printf "\nProgram ended.\n\n"
