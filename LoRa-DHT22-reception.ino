#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <LoRa.h>

int counter = 0;
String data = "";
#define ss D1;
#define reset D2;

#define WIFI_AP         "YOUR_WIFI_AP"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

#define TOKEN           "ESP8266_DEMO_TOKEN"

char thingsboardServer[] = "YOUR_THINGSBOARD_HOST_OR_IP";

WiFiClient wifiClient;
PubSubClient client(wifiClient);
int status = WL_IDLE_STATUS;
unsigned long lastSend;

void setup()
{
  Serial.begin(115200);
  LoRa.setPins(ss, reset);
  while (!Serial);
  Serial.println("LoRa Receiver");
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  delay(10);
  InitWiFi();
  client.setServer( thingsboardServer, 1883 );
  lastSend = 0;
}

void loop()
{
  if ( !client.connected() ) {
    reconnect();
  }

  if ( millis() - lastSend > 1000 ) { // Update and send only after 1 seconds
    getAndSendLoRaData();
    lastSend = millis();
  }

  client.loop();
}

void getAndSendLoRaData()
{
  data = "";
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Received packet: ");
    while (LoRa.available()) {
      //Serial.print((char)LoRa.read());
      data += (char)LoRa.read();    
  }  
    Serial.print(data);
    Serial.print(" with RSSI: ");
    Serial.println(LoRa.packetRssi());
    
    // Prepare a JSON payload string
    String payload = "{";
    payload += "\"LoRa_DHT22: \":"; payload += data; 
    payload += "}";

    char attributes[100];
    payload.toCharArray( attributes, 100 );
    client.publish( "v1/devices/me/telemetry", attributes);
    Serial.println( attributes );
 }
}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("ESP8266 Device", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}




