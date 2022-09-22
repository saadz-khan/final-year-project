/*The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Network.h>


// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

//const uint64_t pipe_gen = 0xE8E8F0F0E1LL; 
//const uint64_t pipe_atm = 0xE8E8F0F0E1FF; 
RF24 radio(2,4);
RF24Network network(radio); // Include the radio in the network
const uint16_t this_node = 00;
float data[6];
float data1[3];
float data2[3];
/*struct MyData {
  float V;
  float C;
  float P;
  float H;
  float T;
  float S;
};*/
//MyData data;
// Insert your network credentials
#define WIFI_SSID "szk"
#define WIFI_PASSWORD "hello123"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDHv-sXK86jCKyguuzHXoIUNL3M7tF-CYA"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "uqadoos.bee19seecs@seecs.edu.pk"
#define USER_PASSWORD "Seecs1609"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://esp-firebase-demo-103cc-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String tempPath = "/temperature";
String humPath = "/humidity";
String timePath = "/timestamp";
String SolarPath = "/intensity";
String voltPath = "/Voltage";
String currentPath = "/Current";
String powerPath = "/Power";

// Parent Node (to be updated in every loop)
String parentPath;

FirebaseJson json;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Variable to save current epoch time
int timestamp;


//float temperature;
//float humidity;
//float pressure;
//float intensity;

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 300000;


// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function that gets current epoch time
unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}

void setup(){
  Serial.begin(115200);

  Wire.begin();
  Serial.println(F("MASTER NODE Test begin"));
  initWiFi();
  timeClient.begin();

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
  
  //radio.begin();
  //radio.setAutoAck(false);
  //radio.setDataRate(RF24_250KBPS);
  //radio.openReadingPipe(1, pipe_gen);
  //radio.startListening();

  SPI.begin();
radio.begin();
network.begin(90, this_node); //(channel, node address)
radio.setDataRate(RF24_250KBPS);
}

void loop(){
  recvData();
  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    //Get current timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);

    parentPath= databasePath + "/" + String(timestamp);
    
    json.set(humPath.c_str(), String(data1[0]));
    json.set(tempPath.c_str(), String(data1[1]));
    json.set(SolarPath.c_str(), String(data1[2]));
    json.set(voltPath.c_str(), String(data2[0]));
    json.set(currentPath.c_str(), String(data2[1]));
    json.set(powerPath.c_str(), String(data2[2]));
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
}
void recvData()
{
  /*if ( radio.available() ) {
    radio.read(&data, sizeof(MyData));
    }*/

    network.update();
while ( network.available() ) { // Is there any incoming data?
RF24NetworkHeader header;
 
network.read(header, &data, sizeof(data)); // Read the incoming data
 
if (header.from_node == 1) { // If data comes from Node 01
data1[0] = data[0];
data1[1] = data[1];
data1[2] = data[2];
}
 
if (header.from_node == 2) { // If data comes from Node 02
data2[0] = data[3];
data2[1] = data[4];
data2[2] = data[5]; 
}

 
}
/*if (!network.available()){
data1[0] = 0.0f;
data1[1] = 0.0f;
data1[2] = 0.0f;
data2[0] = 0.0f;
data2[1] = 0.0f;
data2[2] = 0.0f; 
  }*/
}
