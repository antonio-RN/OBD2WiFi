# OBD2WiFi (WIP)

## General

**OBD2WiFi** is personal project where vehicle data is read via CAN and displayed in a dashboard. For this purpose, the communication from the car to the display is done in two different steps:

1. Car CAN -> WiFi (UDP socket). The data transmitted through the CAN bus is converted to a readable format using an standard commercial OBD2 WiFi Reader ([like this one](https://www.amazon.com/dp/B07L4926C1/ref=sspa_dk_detail_4?psc=1&pd_rd_i=B07L4926C1&pd_rd_w=jJnMr&pf_rd_p=48d372c1-f7e1-4b8b-9d02-4bd86f5158c5&pd_rd_wg=o44Hh&pf_rd_r=K5BCCWP1WB2TCG8MA84K&pd_rd_r=22d9af68-a7fd-4cb8-8834-dc7e2cdeb4f7&spLa=ZW5jcnlwdGVkUXVhbGlmaWVyPUEzNjRXNzEyS1JRREVHJmVuY3J5cHRlZElkPUEwMTE1Nzc3VDFBVVJWTE02TFI3JmVuY3J5cHRlZEFkSWQ9QTAzODkxNzIyMkdRV0JHSklUWkRXJndpZGdldE5hbWU9c3BfZGV0YWlsJmFjdGlvbj1jbGlja1JlZGlyZWN0JmRvTm90TG9nQ2xpY2s9dHJ1ZQ==)). This device uses the ELM327 microcontroller to interpret CAN messages and is adressed via a WiFi AP created by itself. More info about the format of the inputs / outputs of this microcontroller can be found in its own [datasheet](https://www.elmelectronics.com/wp-content/uploads/2017/01/ELM327DS.pdf).

2. WiFI (UDP socket) -> display. The command requests to the ELM327, retrieval of data in readable format, manipulation and display of the received data is all done in a programmable microcontroller. For this task the ESP8266 (in NodeMCU board [like this one](https://www.amazon.com/ESP8266-microcontroller-NodeMCU-WiFi-CP2102/dp/B071WRD25D/ref=sr_1_6?dchild=1&keywords=nodemcu&qid=1590143570&sr=8-6)) has been chosen, given its built-in WiFi capabilities as well as the amount of process power and I/O ports. Said microcontroller is connected to two LCD IPS 1.3" ST7789 displays ([like this one](https://www.amazon.com/MakerFocus-Display-1-3inch-Interface-Routines/dp/B07P9X3L7M/ref=sr_1_1?dchild=1&keywords=lcd+ips+spi+st7789&qid=1590145691&sr=8-1)) via SPI, where the data is displayed.

In this GitHub project will be shared all the code related to the second step listed above.

## Logic & programming

The logic of the data retreival, processing and display is the following one:
- ESP8266 sends data request command over WiFi to ELM327 (following ELM327 formatting and SAE J1962 standard, more info [here](https://en.wikipedia.org/wiki/OBD-II_PIDs))
- ELM327 converts request to CAN format, vehicle ECU sends back data via CAN and ELM327 converts data to readable format and sends it over WiFi to ESP8266
- ESP8266 converts HEX stream received to numeric values or options (according to ELM327 formatting and SAE J1962 standard)
- ESP8266 sends said results to both ST7789 display controllers via SPI bus, and shows them in a pre-defined disposition

For doing all this work, the ESP8266 will be programmed in the PlatformIO IDE (add-on for Microsoft Visual Code) in Arduino framework, so all the Arduino libraries are available. In particular, three libraries will be used: 
- [ESP8266WiFi](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi) (included in Arduino framework): used to manage the UDP socket connection to the OBD2 WiFi Reader.
- [SPI](https://github.com/esp8266/Arduino/tree/master/libraries/SPI) (included in Arduino framework): used to manage the SPI data bus 
that connects the ESP8266 and the two displays.
- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) (not included in Arduino framework, must be installed via PlatformIO IDE library manager): used to generate the display graphics and send data to the ST7789 display controller.


**WIP**
