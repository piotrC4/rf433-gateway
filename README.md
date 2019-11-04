[Homie](https://github.com/homieiot/homie-esp8266) and ESP8266 based firmware for RF433 gateway.

## Features

* receive RF433 radio message and publish it to broker as MQTT message
* All Homie buildin features (OTA,configuration)

## Installation:

### Requirements

* [Visual Studio Code](https://code.visualstudio.com/)
* [PlatformIO IDE extenstion](https://docs.platformio.org/en/latest/ide/vscode.html)
* [GIT](https://git-scm.com/downloads)

### 1. Clone the Repository into VS Code

In VS Code press F1 enter ''git: clone'' + Enter and insert link to my repository (https://github.com/enc-X/rf433-gateway)

### 2. Build binary file

In **PlatformIO** menu choose **PROJECT TASKS -> Build**

### 3. Upload firmware to ESP8266

Connect ESP to PC via serial adapter. In **PlatformIO** menu choose option **PROJECT TASKS -> Upload**. 

### 4. Installig Homie UI

Copy UI bundle to data/homie folder  (See https://github.com/homieiot/homie-esp8266/tree/develop/data/homie )

### 5. Upload SPIFFS image to ESP8266

In **Platformio** menu choose option **PROJECT TASKS ->Upload File System image**

## Initial configuration

Software is build on top of Homie framework - configuration will be done in Homie-way. Connect to MyIOT-xxxxx AP and go to configration UI. Also there is Andorid configuration app - see https://homieiot.github.io/homie-esp8266/docs/develop/configuration/http-json-api/

## Usage

### Data reading

### Configuration


## Credits

* (https://github.com/mhaack/mqtt-433mhz-gateway-homie)