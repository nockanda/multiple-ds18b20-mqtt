#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//아래에 있는 내용은 당연히 있어야한느 것이다!
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS D3 //몇번핀에 연결했냐
#define TEMPERATURE_PRECISION 12
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress myaddress1,myaddress2,myaddress3;
DeviceAddress mysensors[]={
  {0x28,0x92,0x4B,0x79,0xA2,0x00,0x03,0x38},
  {0x28,0xA9,0x4B,0x95,0xF0,0x01,0x3C,0x79},
  {0x28,0xE2,0xEA,0x95,0xF0,0x01,0x3C,0x0A}
};
//mysensors[0]
//mysensors[1]
//mysensors[2]

float mytemp[3] = {0,0,0};

// Update these with values suitable for your network.

const char* ssid = "nockanda";
const char* password = "11213144";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      //client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  sensors.begin(); //있어야한느거
  if(sensors.getDeviceCount() == 3){
    //발견한 주소를 대입한다!
    sensors.getAddress(myaddress1, 0);  
    sensors.getAddress(myaddress2, 1);  
    sensors.getAddress(myaddress3, 2);

    sensors.setResolution(myaddress1, TEMPERATURE_PRECISION);
    sensors.setResolution(myaddress2, TEMPERATURE_PRECISION);
    sensors.setResolution(myaddress3, TEMPERATURE_PRECISION);
  }else{
    //Serial.println("연결되어있지 않거나 2개이상의 센서가 연결되었다!");
  }
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  sensors.requestTemperatures();
  //Serial.println("DONE");

  printTemperature(myaddress1);
  printTemperature(myaddress2);
  printTemperature(myaddress3);

  String myjson = "{\"temp1\":"+String(mytemp[0])+",\"temp2\":"+String(mytemp[1])+",\"temp3\":"+String(mytemp[2])+"}";

  client.publish("nockanda/sensors", myjson.c_str());
}

void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  if(tempC == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
    return;
  }

  //지금 입력받은 주소로 온도값을 측정하는데 누구껀지 확인해보자!
  for(int i =0;i<3;i++){
    if(is_match(deviceAddress,mysensors[i])){
       //match
       //Serial.print(i+1);
       //Serial.println("번째 센서입니다!");
       mytemp[i] = tempC;
       break;
    }
  }
  
  //Serial.print("Temp C: ");
  //Serial.println(tempC);
}

//주소2개를 입력받아서 같은지 아닌지를 반환하는 함수!
bool is_match(DeviceAddress da1, DeviceAddress da2){
  //일단 같다고 보고 시작한다!
  bool result = true;
  for(int i = 0;i<8;i++){
    //단 하나라도 틀리면 틀린것이다!
    if(da1[i] != da2[i]){
      result = false;
      break;
    }
  }

  return result;
}