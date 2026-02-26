# envmon
Simple monitoring device using ESP32 and MQTT  

### Hardware and components
- DOIT ESP32 DEVKIT V1
- OLED SSD1306
- DHT11
- LDR
- Resistor 10k

### How to monitor  
mosquitto_sub -h 192.168.8.192 -p 1885 -t topic/light -F "%I %t %p" >> mqtt-light.log

### How it looks like
![env mon](20251212_164827.jpg)  
  
![env mon](20251212_111833.jpg)  

Tidier version:
![env mon](betterwiring.jpeg)  

### If any case you want to plug the esp32 on a headless PC so you have to compile and upload via linux CLI
Install arduino-cli:
```
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
mv bin/arduino-cli /usr/local/bin/
arduino-cli version
```
Install board:
```
arduino-cli core install esp32:esp32
arduino-cli core list
```
Create new sketch or cd into existing folder containing .ino file
```
arduino-cli sketch new env_mon
cd env_mon
```
Run this command:
```
arduino-cli board list
```
will show something like this:
```
Port         Protocol Type              Board Name FQBN Core
/dev/ttyUSB0 serial   Serial Port (USB) Unknown
```
Compile the code:
```
arduino-cli compile --fqbn esp32:esp32:esp32doit-devkit-v1
```
Install if any missing library
```
arduino-cli lib install "PubSubClient"
arduino-cli lib install "Adafruit GFX Library"
arduino-cli lib install "Adafruit SSD1306"
arduino-cli lib install "DHT sensor library"
```
Then try compile again
and upload (define upload.speed=115200 to prevent too high):
```
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32doit-devkit-v1 --upload-property upload.speed=115200
```
To monitor serial (match baudrate to value defined in the code):
```
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=9600
```
