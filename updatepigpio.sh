#!/bin/bash

current=$(pigpiod -v)
latest=$(curl -s "https://api.github.com/repos/joan2937/pigpio/releases/latest" | jq -r .tag_name | cut -dv -f2)

if [ "$current" -eq "$latest" ]; then
	printf "\nLatest pigpio version (ver. $latest) is already installed.\n\n"
else

	printf "\nRemoving old install files ...\n\n"
	sudo rm -rf ~/work/github/installs/pigpio-master/

	printf "\nDownloading new files ...\n\n"
	wget https://github.com/joan2937/pigpio/archive/master.zip
	unzip master.zip -d ~/work/github/installs/
	sudo rm master.zip
	cd ~/work/github/installs/pigpio-master

	printf "\nInstalling latest pigpio version ...\n\n"
	make
	sudo make install

	printf "\nUpdate finished.\n\n"
fi
