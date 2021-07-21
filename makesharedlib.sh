#!/bin/bash

#https://stackoverflow.com/questions/59895/how-can-i-get-the-source-directory-of-a-bash-script-from-within-the-script-itsel/246128
path="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

gcc -Wall -Werror -pthread -fpic -shared -o $path/R503_fingerprint.so $path/R503_fingerprint.c $path/R503_fingerprint.h -lpigpio -lrt
