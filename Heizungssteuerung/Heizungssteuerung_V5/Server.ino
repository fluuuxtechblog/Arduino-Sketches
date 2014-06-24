/***********************************************************************************************************
  Datei:         Server.ino                                                                                *
  Autor:         Enrico Sadlowski                                                                          *
  Erstellt:      17.05.2013                                                                                *
  Aktualisiert:  06.06.2013                                                                                *
                                                                                                           *
  - Funktionen zum senden der Temperaturdaten aller Sensoren an ein PHP-Script                             *
    das die Werte in eine MySQL-Datenbank schreibt.                                                        *
  - Webformular zum anpassen der Min- und MaxTemperatur-Schaltschwelle für Sensoren die eine Pumpe steuern *
  - Webformular zum anpassen der Haltetemperatur für Sensoren die Mischermotoren steuern                   *
***********************************************************************************************************/





/***************************************************
  Temperaturdaten aller Sensoren an Server senden  *
  Antwort vom Server parsen und darauf reagieren   *
***************************************************/
void sendTemperatureData() {
  if (client.connect(eeprom_config.serverip, eeprom_config.webserverPort)) {
    
    if(DEBUGMODE) {
      Serial.println(F("Verbindung hergestellt"));
      Serial.println(F("Sende Temperaturdaten an Server"));
    }
    
    client.print("GET " + String(SERVERURL));
    client.print("?key="+ String(SERVERKEY));
    client.print("&c=");
    client.print(numSensors);
      
    sensors.requestTemperatures();
      
    for(byte i=0; i<numSensors; i++) {
      int temp = sensors.getTempC(eeprom_config.deviceaddress[i]);
      client.print("&T"+String(i+1)+"="); 
      client.print(temp,1);
    }
          
    client.println(" HTTP/1.1");
    client.println("Host: " + String(SERVERHOST));
    client.println("User-Agent: "+String(USERAGENT));
    client.println("Accept: text/html");
    client.println("Connection: close");
    client.println();    
  } 
  else {
    if(DEBUGMODE) Serial.println(F("Keine Verbindung"));
  }
  
  // Rückgabe des Servers auswerten Text zwischen < und > aus PHP-Datei ausgeben
  while (client.connected()) {
    if(client.available()) {
      char c = client.read();
      if(c==startDelimiter) {
        while(client.connected()) {
          char c = client.read();
          if(c==endDelimiter) break;
          if(DEBUGMODE==1) Serial.print(c);
        } Serial.print("\n");
      }
    }
  }
  
  client.stop();
  client.flush();
  
  if(DEBUGMODE) {
    Serial.println(F("Verbindung beendet"));
    Serial.println();
  }
}





/*
void sendHeader(EthernetClient client) {  
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html"));
  client.println(F("Connection: close"));
  client.println();
  client.println(F("<!DOCTYPE html>"));
  client.println(F("<html><head>"));
  client.println(F("<style type='text/css'>"));
  client.println(F("body {font-family:Verdana;}"));
  client.println(F("table {border: 1px solid gray; border-spacing: 6px; border-style: outset; border-collapse: collapse;}"));
  client.println(F("td {border: 1px solid gray; padding: 6px; border-style: inset;}"));
  client.println(F(".bg1 { background:#222; color:#fff; }"));
  client.println(F(".bg2 { background:#fcfcfc; color:#AAA; }"));
  client.println(F(".center { text-align:center; }"));
  client.println(F("form { display:inline;}"));
  client.println(F("input[type='text'] {width:26px; display:inline-block; padding:6px; font-weight:bold; font-size:14pt;}"));
  client.println(F(".small { color:#ccc; font-size=8pt;}"));
  client.println(F(".shadow { box-shadow: 2px 2px 20px #818181;}"));
  client.println(F(".tempist { font-size:20pt; font-weight:bold; text-align:center; color:black;}"));
  client.println(F(".tempsoll { font-size:20pt; font-weight:bold; text-align:center; color:silver;}"));
  client.println(F(".btn {background-color:#79bbff; font-weight: bold; font-size:10pt; display:inline-block; color:#ffffff; padding: 6px 16px;}"));
  client.println(F("</style>"));
  client.println(F("</head><body>"));
}*/
