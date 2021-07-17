#!/bin/bash

./check_rpi_config.sh

rm finger_template.fpt
./makesharedlib.sh
./testcpython.py

printf "\n\tfile (bash):\n"
cat finger_template.fpt
printf "\n"
