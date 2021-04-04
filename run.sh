#!/bin/bash

printf "\nStarting program ...\n\n"

gcc -Wall -pthread -o main main.c fingerprint.c fingerprint.h -lpigpio -lrt && ./main && rm main

printf "\nProgram ended.\n\n"
