#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define exhaust D5
#define manureCollector D6
int gas_sensor = A0; //Sensor pin

float m = -0.263; //Slope
float b = 0.42; //Y-Intercept

const char* ssid = "";
const char* password = "";

//DEVICE CREDENTIALS
#define ORG ""
#define DEVICE_TYPE ""
#define DEVICE_ID ""
#define TOKEN ""
String command;

#include "DHT.h"
#define DHTPIN D2    // what pin we're connected to
#define DHTTYPE DHT11   // define type of sensor DHT 11
DHT dht (DHTPIN, DHTTYPE);
#define light D4

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char pubtopic[] = "iot-2/evt/ppm/fmt/json";
char topic[] = "iot-2/cmd/data/fmt/String";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;
//Serial.println(clientID);

WiFiClient wifiClient;
void callback(char* topic, byte* payload, unsigned int payloadLength);
PubSubClient client(server, 1883, callback, wifiClient);
void setup() {
  pinMode(gas_sensor, INPUT); //Set gas sensor as input
  pinMode(exhaust,OUTPUT); //  exhaust should be switched on or off
  Serial.begin(115200);
  dht.begin();

  Serial.println();
  wifiConnect();
  mqttConnect();
}

void loop() {
    delay(5000);
    delay(5000);
// ammonia levels
      float sensor_volt; //Define variable for sensor voltage
      float RS_gas; //Define variable for sensor resistance
      float ratio; //Define variable for ratio
      float R0; //Define variable for R0
      float sensorValue = analogRead(gas_sensor); //Read analog values of sensor
      sensor_volt = sensorValue*(5.0 / 1023.0); //Convert analog values to voltage
      RS_gas = ((5.0 * 10.0) / sensor_volt) - 10.0; //Get value of RS in a gas
      R0 = RS_gas/3.6; //Calculate R0 
      ratio = RS_gas / R0; // Get ratio RS_gas/RS_air

      double ppm_log = (log10(ratio) - b) / m; //Get ppm value in linear scale according to the the ratio value
      double val = pow(10, ppm_log); //Convert ppm value to log scale
      Serial.print("Ammonia value = ");
      Serial.println(val);//print the ppm percent value to serial
    
      if(val>= 20){
          Serial.print("Birds maybe in Danger");   }    
          
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  Serial.print("Ammonia in PPM: ");
  Serial.println(val);
  Serial.print("Humidity: ");
  Serial.println(h);
  Serial.print("Temperature: ");
  Serial.println(t);
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  if (!client.loop()) {
    mqttConnect();
  }
  PublishData(t,h,val);
delay(100);
}

void wifiConnect() {
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
}

void mqttConnect() {
  if (!client.connected()) {
    Serial.print("Reconnecting MQTT client to "); Serial.println(server);
    while (!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }
    initManagedDevice();
    Serial.println();
  }
}

void initManagedDevice() {
  if (client.subscribe(topic)) {
    Serial.println("subscribe to cmd OK");
  } else {
    Serial.println("subscribe to cmd FAILED");
  }
}

void callback(char* topic, byte* payload, unsigned int payloadLength) {
  Serial.print("callback invoked for topic: "); Serial.println(topic);

  for (int i = 0; i < payloadLength; i++) {
    //Serial.println((char)payload[i]);
    command += (char)payload[i];
  }
Serial.println(command);
if(command == "on"){
   digitalWrite(exhaust,HIGH);
  digitalWrite(manureCollector,HIGH);
  Serial.println("Exhaust and Manure Collector are ON");
  
}
else if(command == "off"){
  Serial.println("Exhaust and Manure Collector are OFF");  
  digitalWrite(exhaust,LOW);
  digitalWrite(manureCollector,LOW);
}
command ="";
}

void PublishData(float t,float h,float val){
 if (!!!client.connected()) {
 Serial.print("Reconnecting client to ");
 Serial.println(server);
 while (!!!client.connect(clientId, authMethod, token)) {
 Serial.print(".");
 delay(500);
 }
 Serial.println();
 }
  String payload = "{\"d\":{\"temperature\":";
  payload += t;
  payload+="," "\"humidity\":";
  payload += h;
  payload+="," "\"val\":";
  payload += val;
  payload += "}}";
 Serial.print("Sending payload: ");
 Serial.println(payload);
  
 if (client.publish(pubtopic, (char*) payload.c_str())) {
 Serial.println("Publish ok");
 } else {
 Serial.println("Publish failed");
 }
}
