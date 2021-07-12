#!/bin/bash

rm finger_template.fpt
./testcpython.py

printf "\n\tfile (bash):\n"
cat finger_template.fpt
printf "\n"
