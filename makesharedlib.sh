#!/bin/bash

gcc -Wall -Werror -pthread -fpic -shared -o r503_fingerprint.so external.c r503_fingerprint.c r503_fingerprint.h -lpigpio -lrt
