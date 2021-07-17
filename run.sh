#!/bin/bash

./check_rpi_config.sh

printf "\nStarting program ...\n\n"

gcc -Wall -Werror -pthread -o main main.c r503_fingerprint.c r503_fingerprint.h -lpigpio -lrt && ./main && rm main

printf "\nProgram ended.\n\n"

printf "Finger template:\n"
cat finger_template.fpt
printf "\n\n"
