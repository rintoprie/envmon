#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define I2C_SDA_PIN 26 // Example: Using GPIO 4 for SDA
#define I2C_SCL_PIN 25 // Example: Using GPIO 5 for SCL
#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 64 // OLED height, in pixels
#define OLED_RESET      -1
#define SCREEN_ADDRESS  0x3C
#define LDR_PIN 33
#define DHT11PIN 27
#define SW_PIN 13

TwoWire I2Cbus = TwoWire(0);
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Cbus, OLED_RESET);
DHT dht(DHT11PIN, DHT11);

// Konfigurasi WiFi  
const char* ssid = "";  
const char* password = "";  
String ipaddr;

// Konfigurasi MQTT  
const char* mqtt_server = "192.168.8.192"; // Change to local IP where Mosquitto running
const int mqtt_port = 1885;

WiFiClient espClient;  
PubSubClient client(espClient);  

unsigned long lastPublish = 0;
unsigned long lastWifiAttempt = 0;
unsigned long PUBLISH_INTERVAL = 10000; // 10s, change as needed
int swState = 0;      // Variable to store the button's state

void setup_wifi() {  
  delay(10);  
  Serial.println("Connecting to WiFi...");  

  WiFi.mode(WIFI_STA);        // station mode
  WiFi.setSleep(false);       // IMPORTANT: disable modem sleep
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);  

  while (WiFi.status() != WL_CONNECTED) {  
    delay(500);  
    Serial.print(".");  
  }

  ipaddr = WiFi.localIP().toString();
  Serial.println("\nConnected to WiFi!"); 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(ipaddr);
  delay(500) ;
}  

void ensureWifi() {
  if (WiFi.status() == WL_CONNECTED) return;

  unsigned long now = millis();
  if (now - lastWifiAttempt < 5000) return;  // retry every 5 seconds

  lastWifiAttempt = now;

  Serial.println("WiFi lost, reconnecting...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
}

void reconnectMqtt() {  
  while (!client.connected()) {  
    Serial.println("Attempting MQTT connection...");  
    String clientId = "upj_inf-orion-";
    // clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {  
      Serial.println("Connected to broker!");  
    } else {  
      Serial.print("failed, rc=");  
      Serial.print(client.state());  
      Serial.println(" try again in 5 seconds");  
      delay(5000);  
    }  
  }  
}  

void setup() {
  Serial.begin(9600);
  pinMode(SW_PIN, INPUT_PULLUP);

  I2Cbus.begin(I2C_SDA_PIN, I2C_SCL_PIN, 100000);
  // initialize OLED display with I2C address 0x3C
  if (!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1);
  }
  delay(2000);              // wait two seconds for initializing
  oled.clearDisplay();      // clear display
  oled.setTextColor(WHITE); // set text color
  oled.setTextSize(2);      // set text size
  oled.setCursor(0, 2);     // set position to display (x,y)
  oled.println("env");      // set text
  oled.println("mon");      // set text
  oled.display();           // display on OLED
  delay(500);               // wait half second for initializing

  setup_wifi();  
  client.setServer(mqtt_server, mqtt_port);  

  analogSetAttenuation(ADC_11db);
  dht.begin();
}

void loop() {
  ensureWifi();
  if (!client.connected()) {  
    reconnectMqtt();  
  }  
  client.loop();  // MUST be called frequently
  
  swState = digitalRead(SW_PIN);
  if (swState == LOW) {
    PUBLISH_INTERVAL = 60000; // 60s when jumper connected to GND
  } else {
    PUBLISH_INTERVAL = 10000; // 10s when jumper released
  }

  float humi = dht.readHumidity();
  float temp = dht.readTemperature();
  int lightIntensity = analogRead(LDR_PIN);

  // Menampilkan nilai intensitas cahaya ke Serial Monitor
  oled.clearDisplay(); // clear display
  oled.setTextSize(1);  // set text size
  oled.setCursor(0, 0);  // set position to display (x,y)
  oled.println(ipaddr);
  oled.println("--------------------");
  oled.setCursor(0, 16);     // TEMP
  oled.setTextSize(1);
  oled.print("Temp: ");
  oled.setCursor(0, 26);
  oled.setTextSize(2);
  oled.print(String(temp).c_str());
  oled.setCursor(64, 16);     // HUMI
  oled.setTextSize(1);
  oled.print("Humid: ");
  oled.setCursor(64, 26);
  oled.setTextSize(2);
  oled.print(String(humi).c_str());
  oled.setTextSize(1);
  oled.setCursor(0, 42);    // LIGHT
  oled.print("Light: ");
  oled.setCursor(64, 42);
  oled.setTextSize(2);
  oled.print(lightIntensity);
  oled.display();

  unsigned long now = millis();
  if (now - lastPublish > PUBLISH_INTERVAL) {
    lastPublish = now;      
    String tempStr = String(temp);
    String humiStr = String(humi);
    String lightStr = String(lightIntensity);
    client.publish("upj_inf/orion/temperature", tempStr.c_str());   // Send data to MQTT
    client.publish("upj_inf/orion/humidity", humiStr.c_str());  
    client.publish("upj_inf/orion/light", lightStr.c_str());  
    Serial.println("Data terkirim!");  
  }

  delay(500);
}
