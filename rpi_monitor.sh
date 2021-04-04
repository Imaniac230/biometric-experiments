#!/bin/bash

watch -n 1 "grep . /sys/class/thermal/thermal_zone0/temp && grep . /sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq"
