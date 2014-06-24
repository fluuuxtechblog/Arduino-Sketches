/*
  Projekt: Beleuchtung für Chinchillakäfig
  Autor: Enrico Sadlowski
  Datum: 12.10.2012
  Beschreibung:
  ------------
  Die Beleuchtung verfügt über 4 Modi, die über den Button eingeschaltet werden.
  Modus1 = Einschalten in voller Helligkeit
  Modus2 = Einschalten, Helligkeit über Poti einstellbar
  Modus3 = Einschalten, Helligkeit je nach Lichtintensität der Umgebung (Je Heller desto dunkler die LEDs)
  Modus4 = Ausschalten
  Beim starten des Arduino durch anschließen der Energieversorgung, wird Modus 4 gestartet. (LEDs aus) 
*/

const int ledCount = 6;
const int ledPin[] = { 3, 5, 6, 9, 10, 11 };

const int pushButtonPin = 2;
const int lightSensorPin = 1;
const int potiPin = 0;

int buttonPushCount = 0;
int buttonValue = 0;
int lightValue = 0;
int potiValue = 0;


void setup()
{
  pinMode(pushButtonPin, INPUT);
  pinMode(lightSensorPin, INPUT);
  pinMode(potiPin, INPUT);
  
 Serial.begin(9600); 
}





void loop()
{
  buttonValue = digitalRead(pushButtonPin);
  if (buttonValue == HIGH) 
  {
    buttonPushCount++;
    delay(500);
  }
    
  if(buttonPushCount > 3)
  {
     buttonPushCount = 0; 
  }
  
  
  switch (buttonPushCount) 
  {
    case 1:
      ledOn();
    break;
    case 2:
      ledPoti();
    break;
    case 3:
      ledLightIntensity();
    break;
    default:
      ledOff();
    break;
  }    
}



void ledPoti()
{
  potiValue = analogRead(potiPin) / 4;
  Serial.println("potiValue: "+String(potiValue)); 
  
  for(int i=0; i<ledCount; i++)
  {
    analogWrite(ledPin[i], potiValue);
  }
}



void ledLightIntensity()
{
  lightValue = analogRead(lightSensorPin) / 4;
  Serial.println("lightintensity: "+String(lightValue)); 
  
  if(lightValue < 0) lightValue = 0;
  if(lightValue > 255) lightValue = 255;
  
  for(int i=0; i<ledCount; i++)
  {
    analogWrite(ledPin[i], lightValue);
  }
}




void ledOn()
{
  Serial.println("LEDs On"); 
  for(int i=0; i<ledCount; i++)
  {
    analogWrite(ledPin[i], 255);
  }
}



void ledOff()
{
  Serial.println("LEDs Off"); 
  for(int i=0; i<ledCount; i++)
  {
    analogWrite(ledPin[i], 0);
  }
}
