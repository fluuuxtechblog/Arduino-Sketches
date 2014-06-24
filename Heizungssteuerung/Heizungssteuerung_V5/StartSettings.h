boolean DEBUGMODE  = true;  // Meldungen ausgeben
boolean FIRSTSTART = false;  // Erster Start


#define SERIAL_BAUD 9600




/***********************************************
  Einstellungen für ds1820 Temperatursensoren  * 
***********************************************/
const int DS1820Pin  = 7;                // Pin an dem die Sensoren angeschlossen sind.
int mixerControlMeasureInterval    = 10; // Messinterval für Mischermotoren in Sekunden
int mixerControlSwitchTimeInterval = 2;  // Zeit in Sekunden wie lange ein Pin auf HIGH gezogen werden soll (Steuern des Mischermotors)



/*****************************************************
  Netzwerksettings für EthernetShield und Webserver  *
*****************************************************/
char SERVERHOST[] = {"fluuux.de"};                           // Serverhost auf dem die PHP-Datei liegt
char SERVERURL[]  = {"/ARDUINO/SaveTempToMySQLNEU.php"};     // Pfad zur PHP Datei zum empfangen der Sensordaten
char SERVERKEY[]  = {"85621"};                               // ServerKey zur Überprüfung in PHP-Datei
char USERAGENT[]  = {"Arduino"};                             // Useragent zur Überprüfung in PHP-Datei
unsigned long UPLOADINTERVAL = 25;                           // Uploadinterval in Sekunden













/***********************************************************************************************************************************
  Ab hier nichts mehr ändern  -    Ab hier nichts mehr ändern  -    Ab hier nichts mehr ändern  -    Ab hier nichts mehr ändern    *
***********************************************************************************************************************************/
EthernetClient client;
EthernetServer server(80);

long previousMillis = 0;                  // will store last time was updated
const char startDelimiter = '<';
const char endDelimiter   = '>'; 

long prevMixerControllMillis      = 0;   // Mixercontrol Interval
long prevMixerControlSwitchMillis = 0;   // Schaltinterval der Mischermotoren

const int MAX_PAGENAME_LEN=8;
char buf[MAX_PAGENAME_LEN+1];
