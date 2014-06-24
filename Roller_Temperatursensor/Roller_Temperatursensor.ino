/*
  Projekt:  Temperaturüberwachung für den GOVECS-Roller
  Autor:    Enrico Sadlowski
  Datum:    12.10.2012
  Beschreibung:
  -------------
  Beim starten des Rollers ist die Hintergrundbeleuchtung des LCD ist ausgeschaltet.
  Beim drücken des Buttons, wird das LCD ein- oder ausgeschaltet. 
  Wenn die Aussentemperatur unterschritten oder die Akkumaximaltemperatur überschritten wird, wird das LCD eingeschaltet
  wenn es zuvor aus war. Zusätzlich wird im Display eine Meldung (ACHTUNG!!!) oder (Glatteisgefahr)
  und darunter die Temperatur des Sensors der den Alarm gemeldet hat angezeigt. Sobald die Temperatur im normalen Bereich ist,
  nimmt das Display wieder den Status an, den es vor der Alarmmeldung hatte (Ein- oder ausgeschaltet)
*/


#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 7, 6, 5, 4, 3);   // Digitale Pins für Display

const int lcdLight    = 2;                 // Digitaler Poin an dem die Hintergrundbeleuchtung (+5V) angeschlossen ist  
const int buttonPin   = A3;                 // Digitaler Pin an dem der PushButton angeschlossen ist
OneWire  ds(13);                          // Digitaler Pin für Temperatursensoren
 
DeviceAddress sensor1 = { 0x10, 0x11, 0x40, 0xC0, 0x1, 0x8, 0x0, 0x9A };
DeviceAddress sensor2 = { 0x10, 0x16, 0x2E, 0x57, 0x2, 0x8, 0x0, 0x2F };
 
char sensor1Name[]    = "Akku:   ";
char sensor2Name[]    = "Aussen: ";

int AchtungAussen     = 5;      // Bei Unterschreitung dieser Aussen-Temperatur Meldung ausgeben, Rote LED blinken (Glatteisgefahr)
int AchtungAkku       = 30;      // Bei Überschreitung dieser Akku-Temperatur Meldung ausgeben, Rote LED blinken (Überhitzungsgefahr)

int buttonState       = 0;       // current state of the button
int lastButtonState   = 0;       // previous state of the button
boolean displayState  = false;




void setup() 
{
  pinMode(lcdLight, OUTPUT);
  pinMode(buttonPin, INPUT);  
  
  digitalWrite(lcdLight,HIGH);    // LCD-Hintergrundbeleuchtung einschalten
 
  Serial.begin (9600);
  Serial.flush ();
   
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("GOVECS");
  lcd.setCursor(0, 1);
  lcd.print("Temperatursensor");
  
  delay(10000);
  digitalWrite(lcdLight, LOW); //LCD ausschalten
  lcd.clear();
}




void loop() 
{
  float temp1 = getTemperature(sensor1);
  float temp2 = getTemperature(sensor2);
  
  buttonState = analogRead(buttonPin); //wenn am digitalen Pin dann digitalRead
  
  if(buttonState == 1023) //Bei analogRead 1023 bei digitalRead = HIGH oder LOW
  {
    if(displayState == true) //Wenn Display an, dann ausschalten sonst einschalten
    {
      displayState = false; 
      
    }
    else
    {
      displayState = true; 
      digitalWrite(lcdLight, HIGH); //Display einschalten
    }
  }
 
  if(temp2 < AchtungAussen)      // Temp2 = Aussentemperatur
    LCDAussenTemperaturAlarm();
  else if(temp1 > AchtungAkku)   // Temp1 = Batterietemperatur
    LCDAkkuAlarm();
  else
    LCDTemperaturAnzeige();
}





/***************************************************************
  Wenn die Maximaltemperatur des Akkus überschritten wird,     *
  LCD einschalten wenn aus,                                    *      
  Rote LED blinken lassen                                      *
  Alarmmeldung (ACHTUNG!!!) und Akkutemperatur im LCD ausgeben *
***************************************************************/
void LCDAkkuAlarm()
{
  float temp1 = getTemperature(sensor1);  
  
  if(displayState == false) digitalWrite(lcdLight, HIGH);  

  while(temp1 > AchtungAkku)
  {
    temp1 = getTemperature(sensor1); 
    
    lcd.setCursor(0, 0);
    lcd.print("ACHTUNG!!!      ");
    lcd.setCursor(0, 1);
    lcd.print(sensor1Name);
    lcd.print(temp1);
    lcd.write(char(0xDF));
    lcd.print("C");
    lcd.print("                 ");
    
    digitalWrite(lcdLight, LOW);
    delay(300);
    digitalWrite(lcdLight, HIGH);
    delay(500);
  }
}





/************************************************************************
  Wenn die Aussentemperatur unter einen vorgegebenen Wert fällt,        *
  LCD einschalten wenn aus,                                             *
  Rote LED blinken lassen                                               *
  Alarmmeldung (Glatteisgefahr!!) und Aussentemperatur im LCD ausgeben  *
************************************************************************/
void LCDAussenTemperaturAlarm()
{
  float temp2 = getTemperature(sensor2);

  if(displayState == false) digitalWrite(lcdLight, HIGH);
    
  while(temp2 > AchtungAussen)
  {
    temp2 = getTemperature(sensor1);   
    
    lcd.setCursor(0, 0);
    lcd.print("GLATTEISGEFAHR!!");
    lcd.setCursor(0, 1);
    lcd.print(sensor2Name);
    lcd.print(temp2);
    lcd.write(char(0xDF));
    lcd.print("C");
    lcd.print("                 ");
    
    digitalWrite(lcdLight, LOW);
    delay(300);
    digitalWrite(lcdLight, HIGH);
    delay(500);
  }
}





/*********************************************************************
  Aussentemperatur und Akkutemperatur untereinander im LCD anzeigen  *
*********************************************************************/
void LCDTemperaturAnzeige()
{
  float temp1 = getTemperature(sensor1);
  float temp2 = getTemperature(sensor2);
  
  if(displayState == true) 
    digitalWrite(lcdLight, HIGH);
  else 
    digitalWrite(lcdLight, LOW);
    
  lcd.setCursor(0, 0);
  lcd.print(sensor1Name);
  lcd.print(temp1);
  lcd.write(char(0xDF));
  lcd.print("C");
    
  lcd.setCursor(0, 1);
  lcd.print(sensor2Name);
  lcd.print(temp2);
  lcd.write(char(0xDF));
  lcd.print("C"); 
}









void writeTimeToScratchpad(byte* address)
{
  //reset the bus
  ds.reset();
  //select our sensor
  ds.select(address);
  //CONVERT T function call (44h) which puts the temperature into the scratchpad
  ds.write(0x44,1);
  //sleep a second for the write to take place
  delay(1000);
}





 
void readTimeFromScratchpad(byte* address, byte* data)
{
  //reset the bus
  ds.reset();
  //select our sensor
  ds.select(address);
  //read the scratchpad (BEh)
  ds.write(0xBE);
  for (byte i=0;i<9;i++){
    data[i] = ds.read();
  }
}
 
 
 
 
 
 
 
/******************************************************************
  Temperatur des Sensors auslesen dessen Adresse übergeben wurde  *
******************************************************************/ 
float getTemperature(byte* address)
{
  int tr;
  byte data[12];
 
  writeTimeToScratchpad(address);
 
  readTimeFromScratchpad(address,data);
 
  //put in temp all the 8 bits of LSB (least significant byte)
  tr = data[0];
 
  //check for negative temperature
  if (data[1] > 0x80)
  {
    tr = !tr + 1; //two's complement adjustment
    tr = tr * -1; //flip value negative.
  }
 
  //COUNT PER Celsius degree (10h)
  int cpc = data[7];
  //COUNT REMAIN (0Ch)
  int cr = data[6];
 
  //drop bit 0
  tr = tr >> 1;
 
  return tr - (float)0.25 + (cpc - cr)/(float)cpc;
}
