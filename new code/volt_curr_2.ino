
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <RF24Network.h>

const uint16_t this_node = 02; // Address of this node in Octal format
const uint16_t node00 = 00;
float data[6];
/* 0- General */
RF24 radio(3,4); 
const uint64_t pipe1 = 0xF0F0F0F0AA;
        int decimalPrecision = 2;                   
        int VoltageAnalogInputPin = A1;             
        float voltageSampleRead  = 0;              
        float voltageLastSample  = 0;             
        float voltageSampleSum   = 0;               
        float voltageSampleCount = 0;               
        float voltageMean ;                          
        float RMSVoltageMean ;                      
        float adjustRMSVoltageMean;
        float FinalRMSVoltage;                      
 
        
        float voltageOffset1 =0.00 ;          // to Offset deviation and accuracy. Offset any fake current when no current operates. 
                                                    // Offset will automatically callibrate when SELECT Button on the LCD Display Shield is pressed.
                                                    // If you do not have LCD Display Shield, look into serial monitor to add or minus the value manually and key in here.
                                                    // 26 means add 26 to all analog value measured.
        float voltageOffset2 = 0.00;          // too offset value due to calculation error from squared and square root 

              
const int sensorIn = A0;
int mVperAmp = 100; // use 100 for 20A Module and 66 for 30A Module

double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;
struct sensor
{
  float hum;
  float temp;
  float I;
  byte  NodeNum;
};
sensor dht11_data;

void setup() {
 
/* 0- General */

    Serial.begin(9600);                             /* In order to see value in serial monitor */
  Wire.begin();
  //radio.begin();
  //radio.setAutoAck(false);
  //radio.setDataRate(RF24_250KBPS);
  //radio.openWritingPipe(pipeOut);
  SPI.begin();
  radio.begin();
  radio.openWritingPipe(pipe1);
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.stopListening();
  //------------------------------
  dht11_data.NodeNum = 2;
}
     
void loop() 

{
  
  
   /* 1- AC Voltage Measurement */
        if(micros() >= voltageLastSample + 1000 )                                                                      /* every 0.2 milli second taking 1 reading */
          {
            voltageSampleRead = (analogRead(VoltageAnalogInputPin)- 512)+ voltageOffset1;                             /* read the sample value including offset value*/
            voltageSampleSum = voltageSampleSum + sq(voltageSampleRead) ;                                             /* accumulate total analog values for each sample readings*/
            
            voltageSampleCount = voltageSampleCount + 1;                                                              /* to move on to the next following count */
            voltageLastSample = micros() ;                                                                            /* to reset the time again so that next cycle can start again*/ 
          }
        
        if(voltageSampleCount == 1000)                                                                                /* after 4000 count or 800 milli seconds (0.8 second), do the calculation and display value*/
          {
            voltageMean = voltageSampleSum/voltageSampleCount;                                                        /* calculate average value of all sample readings taken*/
            RMSVoltageMean = (sqrt(voltageMean))*1.5;                                                                 // The value X 1.5 means the ratio towards the module amplification.      
            adjustRMSVoltageMean = RMSVoltageMean + voltageOffset2;                                                   /* square root of the average value including offset value */                                                                                                                                                       /* square root of the average value*/                                                                                                             
            FinalRMSVoltage = RMSVoltageMean + voltageOffset2;                                                        /* this is the final RMS voltage*/
            if(FinalRMSVoltage <= 2.5)                                                                                /* to eliminate any possible ghost value*/
            {FinalRMSVoltage = 0;}
      
            voltageSampleSum =0;                                                                                      /* to reset accumulate sample values for the next cycle */
            voltageSampleCount=0;   
             Voltage = getVPP();
 VRMS = (Voltage/2.0) *0.707;  //root 2 is 0.707
 AmpsRMS = ((VRMS * 1000)/mVperAmp)-0.12;
dht11_data.hum = FinalRMSVoltage;
dht11_data.temp = AmpsRMS;


 Serial.print(" The Voltage RMS value is: ");
 Serial.print(dht11_data.hum,decimalPrecision);
 Serial.println(" V ");
 
 Serial.print(" The Current RMS value is: ");
 Serial.print(dht11_data.temp);
 Serial.println(" A ");
 
 float P = (FinalRMSVoltage * AmpsRMS)/1000;
 dht11_data.I = P;
 Serial.print(" The Power is: ");
 Serial.print( dht11_data.I);
 Serial.println(" KW ");
delay(2000);
 bool ok = radio.write(&dht11_data, sizeof(dht11_data)); 
 Serial.println(ok);
          }

}

  
float getVPP()
{
  float result;
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here
  
   uint32_t start_time = millis();
   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
       readValue = analogRead(sensorIn);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the minimum sensor value*/
           minValue = readValue;
       }
   }
   
   // Subtract min from max
   result = ((maxValue - minValue) * 5)/1024.0;
      
   return result;
 }