#include <EEPROM.h>
#include <SPI.h>
#include <Ethernet.h>
#include "avr/pgmspace.h"
#include "EEPROMAnything.h"
#include <Heizungssteuerung_V5>


#define DEBUG
#define SERIAL_BAUD 9600
#define NAMELEN 5
#define VALUELEN 7

int SAS1 = 29; 			// Startadresse für ersten Sensor in deviceaddress
int SAC1 = 150;                 // Startadresse für ersten Sensor in sensorControl
int AddrSelSensor;              // Adresse des gewählten Sensors (deviceaddress)
int AddrSelSensorControl;       // Adresse des gewählten Sensors (sensorControl)
int NrSelSensor;                // Nr des gewählten Sensors (0 - 14)
const int maxNumSensors = 15;   // Anzahl der Sensoren die maximal angeschlossenw erden können
int numSensors;                // Anzahl der angeschlossenen Sensoren

WebServer * webserver;

P(Config_set) = "<font size=\"6\" color=\"red\">Neue Konfiguration gespeichert!<br>Bitte Arduino neu starten oder Reset-Button benutzen!</font><br>";
P(nurZahlenJS) = "<script type=\"text/javascript\">function numbersOnly(obj){if (isNaN(obj.value))obj.value = obj.value.substr(0, obj.value.length-1);}</script>";
P(nurZahlenField) = "onkeydown=\"numbersOnly(this)\" onkeyup=\"numbersOnly(this)\"";



/* Struktur, die im EEPROM gespeichert wird */
struct config_t
{
    byte config_set;
    byte mac[6];
    byte ip[4];
    byte gateway[4];
    byte subnet[4];
    byte dns_server[4];
    unsigned int webserverPort;
    byte serverip[4];
    unsigned int sensorCount;
    uint8_t deviceaddress[maxNumSensors][8]; 
    uint8_t sensorControl[maxNumSensors][6]; // Werte für SensorNr, 1 oder 2 (1=Pumpe, 2=Motor), MinTemp, MaxTemp, Pin1, Pin2 (Pin2 nur bei Mischer oder sonst 0)
} eeprom_config;





/******************************************************************* 
* Standardwerte für EEPROM hier eintragen.                         *
* Diese Werte werden beim ersten Schreiben ins EEPROM verwendet    *
*******************************************************************/
void set_EEPROM_Default() 
{
    eeprom_config.config_set = 1;
 
    //Standard MAC-Adresse des Ethernet-Shields
    eeprom_config.mac[0]=0x90;  
    eeprom_config.mac[1]=0xA2;
    eeprom_config.mac[2]=0xDA;
    eeprom_config.mac[3]=0x0D;
    eeprom_config.mac[4]=0x86;
    eeprom_config.mac[5]=0x7A;
    
    //Lokale IP Adresse über den die Website auf dem Arduino erreichbar ist
    eeprom_config.ip[0]=192;
    eeprom_config.ip[1]=168;
    eeprom_config.ip[2]=0;
    eeprom_config.ip[3]=111;
  
    //Gateway Adresse des Routers
    eeprom_config.gateway[0]=192;
    eeprom_config.gateway[1]=168;
    eeprom_config.gateway[2]=0;
    eeprom_config.gateway[3]=1;
    
    //SUBNET Maske des Netzwerks
    eeprom_config.subnet[0]=255;
    eeprom_config.subnet[1]=255;
    eeprom_config.subnet[2]=255;
    eeprom_config.subnet[3]=0;

    //DNS Server IP
    eeprom_config.dns_server[0]=192;
    eeprom_config.dns_server[1]=168;
    eeprom_config.dns_server[2]=0;
    eeprom_config.dns_server[3]=1;

    //Webserver Port
    eeprom_config.webserverPort=80;
    
    //IP des Servers zu dem die Temperaturdaten gesendet werden
    eeprom_config.serverip[0]=95;
    eeprom_config.serverip[1]=143;
    eeprom_config.serverip[2]=172;
    eeprom_config.serverip[3]=135;
    
    //Anzahl Sensoren
    eeprom_config.sensorCount = 0;
}





/*************************************
* Lesen der EEPROM-Werte beim Start  *
*************************************/
void read_EEPROM_Settings() {  
  EEPROM_readAnything(0, eeprom_config);
  
  if(eeprom_config.config_set != 1) {
    set_EEPROM_Default();
    EEPROM_writeAnything(0, eeprom_config);
  } 
}





/*********************************************************************************
  Netzwerksettings aus EEPROM auslesen und Ethernet-Shield damit initialisieren  *
*********************************************************************************/
void setupNetwork() 
{
  read_EEPROM_Settings();

  IPAddress ip(eeprom_config.ip[0], eeprom_config.ip[1], eeprom_config.ip[2], eeprom_config.ip[3]);                                               
  IPAddress gateway (eeprom_config.gateway[0],eeprom_config.gateway[1],eeprom_config.gateway[2],eeprom_config.gateway[3]);                      
  IPAddress subnet  (eeprom_config.subnet[0], eeprom_config.subnet[1], eeprom_config.subnet[2], eeprom_config.subnet[3]);  
  IPAddress dns_server  (eeprom_config.dns_server[0], eeprom_config.dns_server[1], eeprom_config.dns_server[2], eeprom_config.dns_server[3]);
  Ethernet.begin(eeprom_config.mac, ip, dns_server, gateway, subnet);
}





void setupServerSettings()
{
  read_EEPROM_Settings();
 
  IPAddress SERVERIP(eeprom_config.serverip[0], eeprom_config.serverip[1], eeprom_config.serverip[2], eeprom_config.serverip[3]);
}





//Sensordaten aus EEPROM lesen, Outputs alle ausschalten
void setupSensorData()
{
  read_EEPROM_Settings();
  numSensors = eeprom_config.sensorCount;
}





/********************************************
* Die Seite index.html zum Client schicken  *
********************************************/
void indexHTML(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  server.httpSuccess();
  if (type == WebServer::HEAD) return;
  server.print(F("<html><head><title>index.html</title></head><body>\n"));
  server.print(F("<h1>Heizungssteuerung V5</h1>In diesem Bereich haben Sie die M&ouml;glichkeit s&auml;mtliche Einstellungen f&uuml;r<br>Netzwerk, Sensoren und Ausg&auml;nge vorzunehmen.<br><br>"));
  server.print(F("<a href=\"setupNet.html\">Netzwerkeinstellungen</a>&nbsp;&nbsp;&nbsp;<a href=\"setupServer.html\">Servereinstellungen</a>&nbsp;&nbsp;&nbsp;<a href=\"selectSensor.html\">Sensoren</a>"));
  server.print(F("</body></html>"));
}





/***************************************************************************************************
* Die Seite errorHTML.html zum Client schicken falls eine nicht vorhandene Seite aufgerufen wurde  *
***************************************************************************************************/
void errorHTML(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  if (type == WebServer::HEAD) return;
  server.print(F("<html><head><title>errorHTML</title></head><body>\n"));
  server.print(F("<h1>FEHLER</h1>Sie haben eine Seite aufgerufen die es nicht gibt!<br><br>"));
  server.print(F("<a href=\"index.html\">Startseite</a>&nbsp;&nbsp;&nbsp;<a href=\"setupNet.html\">Netzwerkeinstellungen</a>&nbsp;&nbsp;&nbsp;<a href=\"setupServer.html\">Servereinstellungen</a>&nbsp;&nbsp;&nbsp;<a href=\"selectSensor.html\">Sensoren</a>"));
  server.print(F("</body></html>"));
}





/*************************************************************************************************************
* Die Seite setupNetHTML an den Client schicken                                                              *
*************************************************************************************************************/
void setupNetHTML(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  boolean params_present = false;
  byte param_number = 0;

  server.httpSuccess();
  if (type == WebServer::HEAD) return;

  if (strlen(url_tail)) {
    while (strlen(url_tail)) {
      rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
      if (rc != URLPARAM_EOS) {
        params_present = true;
        param_number   = atoi(name);
   
        if (param_number >=0 && param_number <=5) { eeprom_config.mac[param_number]=strtol(value,NULL,16); }     // MAC address im EEPROM speichern
        if (param_number >=6 && param_number <=9) { eeprom_config.ip[param_number-6]=atoi(value); }              // Lokale IP-Address im EEPROM speichern
        if (param_number >=10 && param_number <=13) { eeprom_config.subnet[param_number-10]=atoi(value); }       // SUBNET-MASK im EEPROM speichern
        if (param_number >=14 && param_number <=17) { eeprom_config.gateway[param_number-14]=atoi(value); }      // GATEWAY-IP im EEPROM speichern
        if (param_number >=18 && param_number <=21) { eeprom_config.dns_server[param_number-18]=atoi(value); }   // DNS-SERVER im EEPROM speichern
        if (param_number == 22) { eeprom_config.webserverPort=atoi(value); }                                     // WEBServer im EEPROM Port speichern
      }
    } EEPROM_writeAnything(0, eeprom_config);
  }

  if(params_present==true) server.printP(Config_set);
  
  // Website Netzwerksettings ausgeben
  server.print(F("<html><head><title>setupNetHTML</title>"));
  server.printP(nurZahlenJS);
  server.print(F("</head><body>\n"));
  server.print(F("<table>"));
  server.print(F("<tr><td><a href=\"index.html\">Startseite</a>&nbsp;&nbsp;&nbsp;<a href=\"setupServer.html\">Servereinstellungen</a>&nbsp;&nbsp;&nbsp;<a href=\"selectSensor.html\">Sensoren</a></td></tr>"));
  server.print(F("</table><br><br>"));
    
  server.print(F("<form action=\"setupNet.html\" method=\"get\">"));  
  server.print(F("<table>"));
  server.print(F("<tr><td colspan=\"2\"><h2>Netzwerk Einstellungen</h2></td></tr>"));
  server.print(F("<tr><td width=\"100\">Mac-Adresse:</td><td>"));
  for (int a=0;a<6;a++) {
    server.print(F("<input type=\"text\" name=\""));
    server.print(a);
    server.print(F("\" value=\""));
    server.print(eeprom_config.mac[a],HEX);
    server.print(F("\" maxlength=\"2\" size=\"2\">\n"));
  }
  server.print(F("</td></tr>"));
  server.print(F("<tr><td>Lokale IP:</td><td>"));
  for (int a=0;a<4;a++) {
    server.print(F("<input type=\"text\" name=\""));
    server.print(a+6);
    server.print(F("\" value=\""));
    server.print(eeprom_config.ip[a]);
    server.print(F("\" maxlength=\"3\" size=\"3\""));
    server.printP(nurZahlenField);
    server.print(F(">\n"));
  }
  server.print(F("</td></tr>"));
  server.print(F("<tr><td>Subnet Maske:</td><td>"));
  for (int a=0;a<4;a++) {
    server.print(F("<input type=\"text\" name=\""));
    server.print(a+10);
    server.print(F("\" value=\""));
    server.print(eeprom_config.subnet[a]);
    server.print(F("\" maxlength=\"3\" size=\"3\""));
    server.printP(nurZahlenField);
    server.print(F(">\n"));
  }
  server.print(F("</td></tr>"));
  server.print(F("<tr><td>Gateway:</td><td>"));
  for (int a=0;a<4;a++) {
    server.print(F("<input type=\"text\" name=\""));
    server.print(a+14);
    server.print(F("\" value=\""));
    server.print(eeprom_config.gateway[a]);
    server.print(F("\" maxlength=\"3\" size=\"3\""));
    server.printP(nurZahlenField);
    server.print(F(">\n"));
  }
  server.print(F("</td></tr>"));
  server.print(F("<tr><td>DNS:</td><td>"));
  for (int a=0;a<4;a++) {
    server.print(F("<input type=\"text\" name=\""));
    server.print(a+18);
    server.print(F("\" value=\""));
    server.print(eeprom_config.dns_server[a]);
    server.print(F("\" maxlength=\"3\" size=\"3\""));
    server.printP(nurZahlenField);
    server.print(F(">\n"));
  }
  server.print(F("</td></tr>"));
  server.print(F("<tr><td>Port:</td><td>"));
  server.print(F("<input type=\"text\" name=\""));
  server.print(22);
  server.print(F("\" value=\""));
  server.print(eeprom_config.webserverPort);
  server.print(F("\" maxlength=\"5\" size=\"5\""));
  server.printP(nurZahlenField);
  server.print(F(">\n"));
  server.print(F("</td></tr>"));
  server.print(F("<tr><td></td><td><input type=\"submit\" value=\"Einstellungen speichern\"></td></tr>"));    
  server.print(F("</table></form>"));
  server.print(F("</body></html>"));
}






/*************************************************************************************************************
* Die Seite setupServerHTML an den Client schicken                                                           *
*************************************************************************************************************/
void setupServerHTML(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  boolean params_present = false;
  byte param_number = 0;

  server.httpSuccess();
  if (type == WebServer::HEAD) return;

  if (strlen(url_tail)) {
    while (strlen(url_tail)) {
      rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
      if (rc != URLPARAM_EOS) {
        params_present=true;        
        param_number = atoi(name);
        
        if (param_number >=23 && param_number <=26) { eeprom_config.serverip[param_number-23]=atoi(value); } // ServerIP Address im EEPROM speichern
      }
    } EEPROM_writeAnything(0, eeprom_config);
  }

  if(params_present==true) server.printP(Config_set);
  
  
  // Website Serversettings ausgeben
  server.print(F("<html><head><title>setupServerHTML</title>"));
  server.printP(nurZahlenJS);
  server.print(F("</head><body>\n"));
  server.print(F("<table>"));
  server.print(F("<tr><td><a href=\"index.html\">Startseite</a>&nbsp;&nbsp;&nbsp;<a href=\"setupNet.html\">Netzwerkeinstellungen</a>&nbsp;&nbsp;&nbsp;<a href=\"selectSensor.html\">Sensoren</a></td></tr>"));
  server.print(F("</table><br><br>"));

  server.print(F("<form action=\"setupServer.html\" method=\"get\">"));  
  server.print(F("<table>"));  
  server.print(F("<tr><td colspan=2><h2>Online-Server</h2></td></tr>"));
  server.print(F("<tr><td width=\"100\">Server IP:</td><td>"));
  for (int a=0;a<4;a++) 
  {
    server.print(F("<input type=\"text\" name=\""));
    server.print(a+23);
    server.print(F("\" value=\""));
    server.print(eeprom_config.serverip[a]);
    server.print(F("\" maxlength=\"3\" size=\"3\""));
    server.printP(nurZahlenField);
    server.print(F(">\n"));
  }
  server.print(F("</td></tr>"));
  server.print(F("<tr><td></td><td><input type=\"submit\" value=\"Einstellungen speichern\"></td></tr>"));    
  server.print(F("</table></form>"));
  server.print(F("</body></html>"));
}





/***********************************************************
* Die Seite selectSensor.html zum Client schicken          *
***********************************************************/
void selectSensor(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  boolean params_present = false;
  byte param_number = 0;

  server.httpSuccess();
  if (type == WebServer::HEAD) return;

  if (strlen(url_tail)) {
    while (strlen(url_tail)) {      
      rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
      if (rc != URLPARAM_EOS) {
        params_present=true;
        param_number = atoi(name);
        
        // DeviceAdresse des gewählten Sensors im EEPROM speichern 
        if (param_number >= AddrSelSensor && param_number <= AddrSelSensor + 7) {
          eeprom_config.deviceaddress[NrSelSensor][param_number - AddrSelSensor] = strtol(value,NULL,16); 
        }
        

        // Sensordaten wie minTemp, maxTemp, HalteTemp, Pin1, Pin2 des gewählten Sensors im EEPROM speichern
        if (param_number >= AddrSelSensorControl && param_number <= AddrSelSensorControl + 5) {          
          eeprom_config.sensorControl[NrSelSensor][param_number - AddrSelSensorControl] = atoi(value);// strtol(value,NULL,16);
        }
      }
    } EEPROM_writeAnything(0, eeprom_config);
  }  
  
  // Website "Sensor auswählen" ausgeben
  server.print(F("<html><head><title>Sensor bearbeiten</title>"));
  server.printP(nurZahlenJS);
  server.print(F("</head><body>\n"));
  server.print(F("<table>"));
  server.print(F("<tr><td><a href=\"index.html\">Startseite</a>&nbsp;&nbsp;&nbsp;<a href=\"setupNet.html\">Netzwerkeinstellungen</a>&nbsp;&nbsp;&nbsp;<a href=\"setupServer.html\">Servereinstellungen</a></td></tr>"));
  server.print(F("</table><br><br>"));

  server.print(F("<form action=\"setupDeviceAddresses.html\" method=\"get\">"));
  server.print(F("<table><tr><td colspan=\"2\" width=\"300\"><h2>Sensor-Einstellungen</h2></td></tr>"));
  server.print(F("<tr><td width=\"100\" align=\"right\"><input type=\"text\" name=\""));
  server.print(27);
  server.print(F("\" value=\""));
  server.print(eeprom_config.sensorCount);
  server.print(F("\" maxlength=\"2\" size=\"2\""));
  server.printP(nurZahlenField);
  server.print(F(">\n"));
  server.print(F("</td><td>Anzahl Sensoren (max 15)</td></tr>"));
  server.print(F("<tr><td align=\"right\">"));
  server.print(F("<select onchange=\"this.form.submit();\" name=\"28\">"));
  server.print(F("<option disabled selected>Bitte ausw&auml;hlen:</option>"));
  for(int i=0; i<maxNumSensors; i++)
  {
    server.print(F("<option value=\""));
    server.print(29+(i*8));
    server.print(F("\">Sensor "));
    server.print(i+1);
    server.print(F("</option>"));
  }
  server.print(F("</select></td><td>W&auml;hlen Sie den Sensor den Sie bearbeiten wollen!</td></tr></table></form>"));
  server.print(F("</body></html>"));
}





/***********************************************************
* Die Seite setupDeviceAddresses.html zum Client schicken  *
***********************************************************/
void setupDeviceAdresses(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  URLPARAM_RESULT rc;
  char name[NAMELEN];
  char value[VALUELEN];
  boolean params_present = false;
  byte param_number = 0;

  server.httpSuccess();
  if (type == WebServer::HEAD) return;

  if (strlen(url_tail)) {
    while (strlen(url_tail)) {
      rc = server.nextURLparam(&url_tail, name, NAMELEN, value, VALUELEN);
      
      if (rc != URLPARAM_EOS) {
        params_present=true;
        param_number = atoi(name);
        
        if (param_number == 27) { eeprom_config.sensorCount=atoi(value); }  // Anzahl der angeschlossenen Sensoren in EEPROM schreiben       
        if (param_number == 28) 
        { 
          AddrSelSensor        = atoi(value);                 // Startadresse des gewählten Sensors aus dem Formular auf (selectSensor.html)
          NrSelSensor          = (AddrSelSensor - SAS1) / 8;  // Nr des gewählten Sensors aus dem Formular auf (selectSensor.html)
          AddrSelSensorControl = SAC1 + (7 * NrSelSensor);    // Adresse der sensorControl des gewählten Sensors aus dem Formular auf (selectSensor.html) errchnen
        }            
      }
    } 
    EEPROM_writeAnything(0, eeprom_config);
    setupSensorData();
  }
  
  if(params_present==true) server.printP(Config_set);

  // Website Sensoreinstellungen ausgeben
  server.print(F("<html><head><title>Sensor Einstellungen</title>"));
  server.printP(nurZahlenJS);
  server.print(F("</head><body>\n"));
  server.print(F("<table>"));
  server.print(F("<tr><td><a href=\"index.html\">Startseite</a>&nbsp;&nbsp;&nbsp;<a href=\"selectSensor.html\">Sensorauswahl</a>&nbsp;&nbsp;&nbsp;<a href=\"setupNet.html\">Netzwerkeinstellungen</a>&nbsp;&nbsp;&nbsp;<a href=\"setupServer.html\">Servereinstellungen</a></td></tr>"));
  server.print(F("</table><br><br>"));

  server.print(F("<form action=\"selectSensor.html\" method=\"get\">"));
  server.print(F("<table width=\"800\"><tr><td colspan=\"2\"><h2>Sensoreinstellungen</h2></td></tr>"));
  server.print(F("<tr><td width=\"120\">Sensor: </td><td>"));
  server.print(NrSelSensor + 1);
  server.print(F("</td></tr>"));
  server.print(F("<tr><td>DeviceAddress</td><td>"));
  for (int a=0;a<8;a++) 
  {
    server.print(F("<input type=\"text\" name=\""));
    server.print(AddrSelSensor+a);
    server.print(F("\" value=\""));
    server.print(eeprom_config.deviceaddress[NrSelSensor][a],HEX);
    server.print(F("\" maxlength=\"2\" size=\"2\">\n"));
  }
  server.print(F("</td></tr><tr><td colspan=\"2\"><hr></td></tr></table>"));
  
  // Formular für Werte für array (sensorControl)
  server.print(F("<table width=\"800\">"));
  server.print(F("<tr><td colspan=\"2\">Bitte geben Sie an was der Sensor steuern soll!<br>"));
  server.print(F("Wenn der Sensor eine <b>W&auml;rmepumpe</b> steuern soll, geben Sie bitte Min-Temp, Max-Temp und Pin1 an.<br>"));
  server.print(F("Soll der Sensor <b>Mischermotoren</b> steuern, geben Sie bitte unter Min-Temp die Temperatur ein,<br>"));
  server.print(F("die durch die Mischermotoren gehalten werden soll.<br>"));
  server.print(F("Geben Sie au&szlig;erdem unter Pin1 den Digitalen Ausgang ein, an dem Mischermotor1 angeschlossen ist<br>"));
  server.print(F("und unter Pin2 den Digitalen Ausgang, an dem Mischermotor2 angeschlossen ist!<br><br></td></tr>"));
  server.print(F("<tr><td colspan=\"2\"><hr></td></tr>"));
  server.print(F("<tr><td width=\"120\"></td><td><input type=\"hidden\" name=\"")); // Sensor Nummer
  server.print(AddrSelSensorControl);
  server.print(F("\" value=\""));
  server.print(NrSelSensor); 
  server.print(F("\"></td></tr>\n")); 
  server.print(F("<tr><td>Steuerung:</td><td><input type=\"radio\" value=\"1\" name=\""));
  server.print(AddrSelSensorControl+1);
  server.print(F("\""));
  if(eeprom_config.sensorControl[NrSelSensor][1] == 1) server.print(F(" checked=\"checked\"")); 
  server.print(F("\"> W&auml;rmepumpe&nbsp;&nbsp;<input type=\"radio\" value=\"2\" name=\""));
  server.print(AddrSelSensorControl+1);
  server.print(F("\""));
  if(eeprom_config.sensorControl[NrSelSensor][1] == 2) server.print(F(" checked=\"checked\"")); 
  server.print(F("\"> Mischermotoren</td></tr>"));  
  server.print(F("<tr><td>Min-Temp:</td><td><input type=\"text\" name=\"")); // Min-Temp-Schaltschwelle
  server.print(AddrSelSensorControl+2);
  server.print(F("\" value=\""));
  server.print(eeprom_config.sensorControl[NrSelSensor][2]);
  server.print(F("\" maxlength=\"2\" size=\"2\""));
  server.printP(nurZahlenField);
  server.print(F("></td></tr>\n"));
    server.print(F("<tr><td>Max-Temp:</td><td><input type=\"text\" name=\"")); // Max-Temp-Schaltschwelle
  server.print(AddrSelSensorControl+3);
  server.print(F("\" value=\""));
  server.print(eeprom_config.sensorControl[NrSelSensor][3]);
  server.print(F("\" maxlength=\"2\" size=\"2\""));
  server.printP(nurZahlenField);
  server.print(F("></td></tr>\n"));
  server.print(F("<tr><td>Pin1:</td><td><input type=\"text\" name=\"")); // Pin1
  server.print(AddrSelSensorControl+4);
  server.print(F("\" value=\""));
  server.print(eeprom_config.sensorControl[NrSelSensor][4]);
  server.print(F("\" maxlength=\"2\" size=\"2\""));
  server.printP(nurZahlenField);
  server.print(F("></td></tr>\n"));
  server.print(F("<tr><td>Pin2:</td><td><input type=\"text\" name=\"")); // Pin2
  server.print(AddrSelSensorControl+5);
  server.print(F("\" value=\""));
  server.print(eeprom_config.sensorControl[NrSelSensor][5]);
  server.print(F("\" maxlength=\"2\" size=\"2\""));
  server.printP(nurZahlenField);
  server.print(F("></td></tr>\n"));
  server.print(F("<tr><td></td><td><input type=\"submit\" value=\"Einstellungen speichern\"></td></tr>")); 
  server.print(F("</table></form>"));
  server.print(F("</body></html>"));
}
