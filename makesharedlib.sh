#!/bin/bash

gcc -Wall -Werror -pthread -fpic -shared -o R503_fingerprint.so R503_fingerprint.c R503_fingerprint.h -lpigpio -lrt
