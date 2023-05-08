#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "DHT.h"
#include <Wire.h>
#include <BH1750.h>
#include <RF24Network.h>
BH1750 lightMeter;

#define DHTPIN 4
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);
RF24 radio(9,10); //  CN and CSN  pins of nrf
const uint16_t this_node = 03; // Address of this node in Octal format
const uint16_t node00 = 00;

//----------------------------------
const uint64_t pipe2 = 0xF0F0F0F066;
//----------------------------------
struct sensor
{
  float hum;
  float temp;
  float I;
  byte  NodeNum;
};
sensor dht11_data;

float data[6];

void setup()
{
  Serial.begin(9600); 
  dht.begin();
  Wire.begin();
  lightMeter.begin();
  SPI.begin();
  radio.begin();
  radio.openWritingPipe(pipe2);
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.stopListening();
  //------------------------------
  dht11_data.NodeNum = 1;
}
void loop()
{
  data[0] = dht.readHumidity();
  data[1] = dht.readTemperature();
  float lux = lightMeter.readLightLevel();
  data[2] = (lux*0.0079);
  if (isnan(data[0]) || isnan(data[1])){
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  dht11_data.hum = data[0];
    dht11_data.temp = data[1];
  dht11_data.I = data[2];
  //-------------------------------------------
  radio.write(&dht11_data, sizeof(dht11_data));
  Serial.print("Humidity: ");
  Serial.println(data[0]);
  Serial.print("Temperature: ");
  Serial.println(data[1]);
  Serial.print("SolarIrradiance: ");
  Serial.println(data[2]);

  delay(20);
}