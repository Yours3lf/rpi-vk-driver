#!/bin/bash

mkdir -p $HOME/.rpi-vk-driver/
mkdir -p $HOME/.local/share/vulkan/icd.d/

DRIVER=librpi-vk-driver.so
JSON=rpi-vk-driver.json

if [ -f "$DRIVER" ]; then
	cp $DRIVER $HOME/.rpi-vk-driver/
else
	cp build/$DRIVER $HOME/.rpi-vk-driver/
fi

if [ -f "$JSON" ]; then
	cp $JSON $HOME/.local/share/vulkan/icd.d/
else
	cp ../$JSON $HOME/.local/share/vulkan/icd.d/
fi
