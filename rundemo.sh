#!/bin/bash

./check_config.sh

printf "\nStarting program ...\n\n"

gcc -Wall -Werror -pthread -o demo demo.c r503_fingerprint.c r503_fingerprint.h -lpigpio -lrt && ./demo && rm demo

printf "\nProgram ended.\n\n"
