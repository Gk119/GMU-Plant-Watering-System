/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial


#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include "dht.h"

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "6990aecfaab343149e353d2ecaa9f042";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "YourSSID";
char pass[] = "password";

// Hardware Serial on Mega, Leonardo, Micro...
#define EspSerial Serial1

// or Software Serial on Uno, Nano...
//#include <SoftwareSerial.h>
//SoftwareSerial EspSerial(2, 3); // RX, TX

// Your ESP8266 baud rate:
#define ESP8266_BAUD 115200

#define motorPin 6
#define DHT11_PIN 8
#define SMD 7
#define SMA A0
#define WATER_TIME 6000

ESP8266 wifi(&EspSerial);
BlynkTimer updateTimer;

WidgetRTC rtc;

unsigned long lwt = 0;  //Last watered time
int smt = 0;        //Soil moisture threshhold

void setup()
{
  // Debug console
  Serial.begin(9600);

  // Set ESP8266 baud rate
  EspSerial.begin(ESP8266_BAUD);
  delay(10);

  pinMode(motorPin,OUTPUT);
  
  Blynk.begin(auth, wifi, ssid, pass);
  updateTimer.setInterval(6000L, UPDATE);
  // You can also specify server:
  //Blynk.begin(auth, wifi, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, wifi, ssid, pass, IPAddress(192,168,1,100), 8080);
  Blynk.syncVirtual(V6);
  delay(2000);
}

void loop()
{
  Blynk.run();
  updateTimer.run(); // Initiates BlynkTimer
}

String currentTime()
{
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  return currentTime;
}

void UPDATE()
{
   DHT();
   SM();
   int soilMoisture=analogRead(SMA);
   int SM=map(soilMoisture,0,1024,0,100);
   SM=100-SM;
   if(SM<smt)
   {
        water(WATER_TIME);
        Blynk.notify(String("Your plant was watered at " + currentTime()).c_str());
    }
 }
 
void DHT()
{
  dht DHT;
    
  int chk = DHT.read11(DHT11_PIN);
  Serial.print("Temperature = ");
  Serial.println(DHT.temperature);
  Serial.print("Humidity = ");
  Serial.println(DHT.humidity);
  if(DHT.temperature>0 && DHT.humidity>0)
  {
    Blynk.virtualWrite(V1,DHT.temperature);
    Blynk.virtualWrite(V2,DHT.humidity);
    Serial.println("Graphed");
  }
}

void SM()
{
  int soilMoisture=analogRead(SMA);
  Serial.print("Soil Moisture=");
  Serial.println(soilMoisture);
  int SM=map(soilMoisture,0,1024,0,100);
  SM=100-SM;
  Serial.print("Soil Moisture Percentage=");
  Serial.println(SM);
  Blynk.virtualWrite(V3,SM);
 }

void water(int time)
{
  
    Serial.println("Watering");
    digitalWrite(motorPin,HIGH);
    delay(time);
    digitalWrite(motorPin,LOW);
    lwt= millis() / 1000;
    Serial.print("Last watered upTime = ");
    Serial.println(lwt); 
    Serial.print("Time:\n");
    clockDisplay();
}

BLYNK_WRITE(V6)   //For updateing soil moisture threshhold value
{
  smt = param.asInt();
  Serial.print("Soil Moisture Threshhold = ");
  Serial.println(smt);
}

BLYNK_WRITE(V5)
{
    water(WATER_TIME); 
}

BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
}

 String clockDisplay()
{
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details

  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + "/" + month() + "/" + year();
  Serial.print("Current time: ");
  Serial.print(currentTime);
  Serial.print(" ");
  Serial.print(currentDate);
  Serial.println();
  return String(currentDate + "  " + currentTime);
}

BLYNK_WRITE(V7)        //Refresh
{
    UPDATE();
    Blynk.syncAll();
}
BLYNK_READ(V4)
{
  randomSeed(millis());
  Blynk.virtualWrite(V4,random(0,1000));
}

