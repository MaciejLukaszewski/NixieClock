# Nixie Clock

_Nixie based retro clock with bluetooth synchronization_

Nixie electron tubes based clock, which can work as part of the smart home infrastructure. In addition to the usual
clock function, it can synchronize time with a home time server via the Bluetooth(BLE) interface.
Detailed specification can be found in `PDF` file.

Each files consists:

* ESP32_Server.ino - home test server code (ESP32 microcontroller)  
* NixieSTM.ioc - configuration file of STM32F031K6T6 microcontroller for CubeIDE environment  
* main.c - main `C` language file for the clock (STM32F031K6T6)
* main.h - clock header file
