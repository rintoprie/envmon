# envmon
Simple monitoring platform using ESP32 and MQTT  
  
We use DOIT ESP32 DEVKIT V1

How to monitor  
mosquitto_sub -h 192.168.8.192 -p 1885 -t topic/light -F "%I %t %p" >> mqtt-light.log
