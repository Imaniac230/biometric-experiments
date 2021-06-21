#!/bin/bash

printf "\nStarting program ...\n\n"

gcc -Wall -pthread -o demo demo.c r503_fingerprint.c r503_fingerprint.h -lpigpio -lrt && ./demo && rm demo

printf "\nProgram ended.\n\n"
