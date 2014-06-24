#include <EEPROMex.h>
#include "I2Cdev.h"
#include "Wire.h"
#include "MPU6050_6Axis_MotionApps20.h"

MPU6050 mpu;

static int alarmActivateBtnPin   = 10;   // Pin des Button zum aktivieren des Alarms
static int alarmLedPin           =  7;   // Pin der LED, die signalisiert das der Alarm ausgelöst wurde           
int alarmLedState                = LOW;
long previousMillis              =  0;
const int blinkInterval          = 100;
static int redLed                = 5;
static int greenLed              = 6;
static byte tolerance            = 20;
bool alarmSystemArmed            = false;  // Alarmanlage scharfgeschaltet
bool DEBUGMODE                   = false;  // true = Meldungen über Serial ausgeben
int alarmActivateBtnPinState;
int lastAlarmActivateBtnPinState;


int savedValues[3];                     // Werte in EEPROM beim Scharfschalten der Alarmanlage



// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

int ax,ay,az=0;


volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high




void dmpDataReady() 
{
  mpuInterrupt = true;
}





void setup() 
{
  Wire.begin();
  
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(alarmActivateBtnPin, INPUT);
  pinMode(alarmLedPin, OUTPUT);   
  
  digitalWrite(redLed, HIGH);

  Serial.begin(115200);  
  Serial.println(F("Initializing I2C devices..."));
  mpu.initialize();
  
  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
 
  Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();

  mpu.setXGyroOffset(30);//(220);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(0);//(-85);
  mpu.setZAccelOffset(1688);

  if (devStatus == 0) 
  {
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);
    Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
    attachInterrupt(0, dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
    dmpReady = true;

    packetSize = mpu.dmpGetFIFOPacketSize();
  } 
  else 
  {
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }
}









void loop() 
{
  getAccelgyroData(); //Werte des Gyrosensors abfragen
  
  
  alarmActivateBtnPinState    = digitalRead(alarmActivateBtnPin);
 
  //Alarmanlage scharf oder unscharf schalten 
  if (alarmActivateBtnPinState != lastAlarmActivateBtnPinState) 
  {
    if (alarmActivateBtnPinState == HIGH) 
    {
      if (alarmSystemArmed == false) // Alarmanlage scharf schalten
      {
        if(DEBUGMODE) Serial.println(F("Save current position in EEPROM"));
        writePosToEEPROM(); // Aktuelle Position in EEPROM speichern
        if(DEBUGMODE) Serial.println(F("Alarm system is armed"));
        alarmSystemArmed = true;
        if(digitalRead(redLed == HIGH)) digitalWrite(redLed, LOW);
        digitalWrite(greenLed, HIGH);
      }
      else // Alarmanlage unscharf schalten
      {
        if(DEBUGMODE) Serial.println(F("Save current position in EEPROM"));
        writePosToEEPROM(); // Aktuelle Position in EEPROM speichern
        if(DEBUGMODE) Serial.println(F("Alarm system is disarmed"));
        alarmSystemArmed = false;

        if(digitalRead(greenLed == HIGH)) digitalWrite(greenLed, LOW);
        digitalWrite(redLed, HIGH);
        ledOff();
      }
    }
    lastAlarmActivateBtnPinState = alarmActivateBtnPinState;
  }
  
  
/***************************************************************************
  Wenn die Alarmanlage scharf geschaltet wurde,                            *
  prüfen ob die aktuelle Position des GyroSensors von der im EEPROM        *
  gespeicherte Position abweicht. Wenn diese abweicht dann Alarm auslösen  *
***************************************************************************/   
  if(alarmSystemArmed)
  {  
    alarmMode();
  }
}








/***********************************************************
  ALARMLED blinken lassen oder ALARMTON ausgeben,          *
  wenn sich Position des zu sichernden Gegenstands ändert  *
***********************************************************/
bool alarmMode()
{
  int sumSaved   = savedValues[0] + savedValues[1] + savedValues[2];
  int sumAktuell = ax + ay + az;
  
  if(sumAktuell < sumSaved - tolerance || sumAktuell > sumSaved + tolerance)
  {    
    blinkLED();
  }
  else
  {
    if(alarmLedState == HIGH)
    {
       ledOff();
    } 
  }
}







/**********************************
  Werte des Gyrosensors abfragen  *
**********************************/
void getAccelgyroData()
{
  if (!dmpReady) return;

  while (!mpuInterrupt && fifoCount < packetSize) { }

  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();

  fifoCount = mpu.getFIFOCount();

  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
    mpu.resetFIFO();
    Serial.println(F("FIFO overflow!"));
  } 
  else if (mpuIntStatus & 0x02) {
    while (fifoCount < packetSize) {
      fifoCount = mpu.getFIFOCount();
    }

    mpu.getFIFOBytes(fifoBuffer, packetSize);
        
    fifoCount -= packetSize;
    
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
      
    ax = ypr[0] * 180/M_PI;
    ay = ypr[1] * 180/M_PI;
    az = ypr[2] * 180/M_PI;
      
    if(DEBUGMODE)
    { 
      Serial.print("ypr\t");
      Serial.print(ax);
      Serial.print("\t");
      Serial.print(ay);
      Serial.print("\t");
      Serial.println(az);
    }
  }
}






void blinkLED()
{
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis > blinkInterval) 
  {
    previousMillis = currentMillis;   

    if (alarmLedState == LOW)
      alarmLedState = HIGH;
    else
      alarmLedState = LOW;

    digitalWrite(alarmLedPin, alarmLedState); 
  }
}




void ledOff()
{
  alarmLedState = LOW;
  digitalWrite(alarmLedPin, alarmLedState);
}







void writePosToEEPROM()
{
  EEPROM.writeLong(0,ax);
  EEPROM.writeLong(4,ay);
  EEPROM.writeLong(8,az); 
  
  savedValues[0] = ax;
  savedValues[1] = ay;
  savedValues[2] = az;
}





void readPosFromEEPROM()
{
  savedValues[0] = EEPROM.readLong(0);
  savedValues[1] = EEPROM.readLong(4);
  savedValues[2] = EEPROM.readLong(8);
}
