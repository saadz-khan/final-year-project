#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Network.h>

#define FIREBASE_HOST "homeautomation-6f713-default-rtdb.firebaseio.com" // Your Firebase Project URL goes here without "http:" , "\" and "/"
#define FIREBASE_AUTH "Aah1FYW7z8B4Xt3tLRf96SW9ZgEmoWTtw3QiIXz0"    // Your Firebase Database Secret goes here
#define WIFI_SSID "SFLT"                                             // your WiFi SSID for which your NodeMCU connects
#define WIFI_PASSWORD "123456789"

// Declare the Firebase Data object in the global scope
FirebaseData firebaseData;

RF24 radio(2,4);
const uint16_t this_node = 00;
//------------------------------------------
const uint64_t pipe1 = 0xF0F0F0F0AA;
const uint64_t pipe2 = 0xF0F0F0F066;
//------------------------------------------
float data[6];
float data1[3];
float data2[3];


// Database main path (to be updated in setup with the user UID)
String databasePath;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Variable to save current epoch time
int timestamp;

struct sensor
{
  float hum;
  float temp;
  float I;
  byte  NodeNum;
};
sensor sensorData;

void setup()
{

  Serial.begin(115200);
  Wire.begin();
  wificonnectivity();
  timeClient.begin();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); // connect to firebase
  Firebase.reconnectWiFi(true);
  SPI.begin();
  radio.begin();
  radio.openReadingPipe(1, pipe1);
  radio.openReadingPipe(2, pipe2);
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();
  delay(1000);
}

void loop()
{
  recvData();
  timestamp = getTime();
  databasePath = "/DataGeneration/" + String(timestamp);
  FirebaseSend();
  delay(1000);
}
void wificonnectivity()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); //try to connect with wifi
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);

    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(500);
    }
  Serial.print('\n');
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP()); //print local IP address
}
unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}

void FirebaseSend()
{

  // Write values to Firebase
  Firebase.setString(firebaseData, databasePath + "/Humidity", data1[0]);
  Firebase.setString(firebaseData, databasePath + "/Temperature", data1[1]);
  Firebase.setString(firebaseData, databasePath + "/SolarIntensity", data1[2]);
  Firebase.setString(firebaseData, databasePath + "/Voltage", data2[0]);
  Firebase.setString(firebaseData, databasePath + "/Current", data2[1]);
  Firebase.setString(firebaseData, databasePath + "/Power", data2[2]);
  Firebase.setString(firebaseData, databasePath + "/TimeStamp", String(timestamp));
  Firebase.setString(firebaseData, "/Power", data2[2]);
  Firebase.setString(firebaseData, "/Voltage", data2[0]);

  Serial.println("Writing Value");
}
void checknode(int nodenum){
  float temp[6];
  if (sensorData.NodeNum == 1){
    data1[0] = sensorData.hum;
    data1[1] = sensorData.temp;
    data1[2] = sensorData.I;
  }
  if(sensorData.NodeNum == 2){
    temp[0] = sensorData.hum;
    temp[1] = sensorData.temp;
    temp[2] = sensorData.I;
    if(temp[0] > 0 || temp[1] > 0 || temp[2] > 0){
      data2[0] = temp[0];
      data2[1] = temp[1];
      data2[2] = temp[2];
    }
  }

}
void recvData()
{   
      //--------------------------------------------
  if(radio.available())
  {
    radio.read(&sensorData, sizeof(sensorData));
    Serial.println(sensorData.temp);
    checknode(sensorData.NodeNum);
    radio.read(&sensorData, sizeof(sensorData));
    Serial.println(sensorData.temp);
    checknode(sensorData.NodeNum);
    //------------------------------------------
    digitalWrite(3, HIGH); digitalWrite(2, LOW);
  }
  else
  {
    digitalWrite(3, LOW); digitalWrite(2, HIGH);
  }
    

}