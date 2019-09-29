#!/bin/bash

mkdir -p $HOME/.rpi-vk-driver/
cp librpi-vk-driver.so $HOME/.rpi-vk-driver/
cp libQPUassembler.so $HOME/.rpi-vk-driver/
cp libbrcm.so $HOME/.rpi-vk-driver/
mkdir -p $HOME/.local/share/vulkan/icd.d/
cp rpi-vk-driver.json $HOME/.local/share/vulkan/icd.d/