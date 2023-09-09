#!/bin/bash

sudo usermod -a -G dialout $USER
wget https://bootstrap.pypa.io/get-pip.py 
sudo python get-pip.py 
sudo pip install pyserial 
mkdir -p ~/Arduino/hardware 
cd ~/Arduino/hardware 
git clone https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series.git heltec 
cd heltec/esp32 
git submodule update --init --recursive 
cd tools 
python get.py 
cd ~/Arduino/hardware/heltec/esp8266 
git submodule update --init --recursive 
cd tools 
python get.py

