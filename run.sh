#!/bin/bash

./check_config.sh

printf "\nStarting program ...\n\n"

gcc -Wall -pthread -o main main.c r503_fingerprint.c r503_fingerprint.h -lpigpio -lrt && ./main && rm main

printf "\nProgram ended.\n\n"
