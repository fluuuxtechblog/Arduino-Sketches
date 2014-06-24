/*
  Sketch-Name: Twitter_DHT22_Values
  Autor: Enrico Sadlowski
  Datum: 30.01.2013
  
  Beschreibunhg
  ---------------------
  Es wird eine Statusmitteilung mit der aktuellen Temperatur und Luftfeuchte an
  Twitter gesendet sobald Temperatur oder Luftfeuchte einen vorgegebenen Maximalwert 
  pberschreiten. Wenn dies geschieht wird ausserdem eine LED eingeschaltet die erst 
  ausgeschaltet wird  wenn die Werte für Temperatur und Luftfeuchte unter den 
  Maximalwert fallen.
*/
#include <SPI.h>
#include <Ethernet.h>
#include <Twitter.h>
#include "DHT.h"
 
#define DHTPIN 9         // Digital-Pin an dem der DHT22-Sensor hängt     
#define DHTTYPE DHT22    // Typ des DHT-Sensors (DHT11, DHT21, DHT22)
 
DHT dht(DHTPIN, DHTTYPE);

byte mac[] = { 0x5A, 0xA2, 0xDA, 0x0D, 0x56, 0x7A }; // Mac-Adresse des Ethernet-Shield
Twitter twitter("375802830-ZuzjjuE4Thg9zZLX5SL5yUpgt6sWj4ZZwQJuZ2bK"); // Twitter-Token
const int LEDPin = 7;   // Digital-Pin an der die Status-LED hängt
const int maxTemp = 30; // Maximale Temperatur
const int maxHumi = 75; // Maximale Luftfeuchte
boolean sleep = false;
boolean DEBUG = false;  // true = Serial-Messages ausgeben

int statusRGBLED[] = {3, 5, 6}; // 3 Digital-Pins der Status RGB LED (Rot, Grün, Blau); 
const boolean ON = HIGH; 
const boolean OFF = LOW; 
const boolean RED[] = {ON, OFF, OFF}; 
const boolean GREEN[] = {OFF, ON, OFF}; 
const boolean BLUE[] = {OFF, OFF, ON}; 
const boolean BLACK[] = {OFF, OFF, OFF}; 
const boolean WHITE[] = {ON, ON, ON}; 







void setup()
{
  delay(1000);
  Serial.begin(9600);
  Ethernet.begin(mac);
  dht.begin();
  
  for(int i = 0; i < 3; i++) 
    pinMode(statusRGBLED[i], OUTPUT); 
    
  if(!DEBUG) Serial.println("DEBUG OFF");  
}







void loop()
{
  getDHT22();
  delay(1000);
}








//Temperatur des DHT22 ermitteln
void getDHT22()
{
  int h = dht.readHumidity();     //Luftfeuchte auslesen
  int t = dht.readTemperature();  //Temperatur auslesen
 
  String tempString = String(t);  //Temperatur
  String humiString = String(h);  //Luftfeuchte
  
  //Prüfen ob eine gültige Zahl zurückgegeben wird. Wenn NaN (not a number) zurückgegeben wird, dann Fehler ausgeben.
  if (isnan(t) || isnan(h)) 
  {
    setColor(statusRGBLED, RED);
    if(DEBUG) Serial.println("DHT22 konnte nicht ausgelesen werden");
  } 
  else
  {
    if(DEBUG) Serial.println("Temperatur: " + tempString + char(186) + String("C") + ", Luftfeuchte: " + humiString + "%"); 
    
    if(t > maxTemp || h > maxHumi)
    {
      if(sleep == false)
      {      
        if(DEBUG) Serial.println("@profwebapps | Temperatur: " + tempString + char(186) + String("C") + ", Luftfeuchte: " + humiString + "%"); 
      
        String TweetString = String("@profwebapps | Temperatur: " + tempString + char(186) + String("C") + ", Luftfeuchte: " + humiString + "%");
      
        char TempTweet[TweetString.length() + 1];
 
        TweetString.toCharArray(TempTweet, TweetString.length() + 1);
  
        sendTwitterMessage(TempTweet);
        
        setColor(statusRGBLED, BLUE);
        
        sleep = true;
      }
    }
    else
    {
      if(sleep)
      {
        sleep = false;   
        setColor(statusRGBLED, BLACK); 
        if(DEBUG) Serial.println("Temperatur und Luftfeuchte wieder im normalen Bereich!");
        if(DEBUG) Serial.println("Temperatur: " + tempString + char(186) + String("C") + ", Luftfeuchte: " + humiString + "%"); 
      }
    }
  }
}






//Tweet an Twitter senden 
void sendTwitterMessage(char* tweet)
{
  if(DEBUG) Serial.println("Verbinde zu Twitter...");
  
  
  if (twitter.post(tweet)) 
  {
    int status = twitter.wait();
    if (status == 200) 
    {
      setColor(statusRGBLED, WHITE);
      if(DEBUG) Serial.println("Verbindung hergestellt...");
      delay(2000);
    } 
    else 
    {
      setColor(statusRGBLED, RED);
      if(DEBUG) Serial.print("Fehler: code ");
      if(DEBUG) Serial.println(status);
      delay(5000);
    }
  } 
  else 
  {
    setColor(statusRGBLED, RED); 
    if(DEBUG) Serial.println("Verbindung fehlgeschlagen.");
    delay(5000);
  }
}





//RGB-LED Farbe schalten
void setColor(int* led, const boolean* color)
{
  for(int i = 0; i < 3; i++)
    digitalWrite(led[i], color[i]); 
} 
