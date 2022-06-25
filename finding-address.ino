//아래에 있는 내용은 당연히 있어야한느 것이다!
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2 //몇번핀에 연결했냐
#define TEMPERATURE_PRECISION 12
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress myaddress;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  sensors.begin(); //있어야한느거

  //발견한 총 디바이스의 갯수 지금은 1
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  //단 1개의 온도센서가 연결되었을때 주소값을 확인하겠다!
  if(sensors.getDeviceCount() == 1){
    //발견한 주소를 대입한다!
    sensors.getAddress(myaddress, 0);  
    //주소를 출력한다!
    printAddress(myaddress);
  }else{
    Serial.println("연결되어있지 않거나 2개이상의 센서가 연결되었다!");
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:

}

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    Serial.print(",");
  }
}