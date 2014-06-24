<h1>GetGMail</h1>
Projekt: Eingang neuer eMails im Google Postfach anzeigen<br />
Autor: Enrico Sadlowski<br />
Erstellungsdatum:      27.03.2013<br />
Letzte Aktualisierung: 28.03.2013<br />
<h2>Beschreibung:</h2>
Dieses Sketch wartet auf ankommende Signale über den seriellen Port
Wird ein M empfangen dann fängt eine RGB LED an im Takt zu blinken und
dabei ihre Farbe zu ändern. Wenn ein N empfangen wird, dann wird die LED 
abgeschaltet.
  
Zu diesem Script gehört das Python-Script check-gmail.py das minütlich durch einen
CronJob aufgerufen wird. Das Python-Script  verbindet sich zum Google-Mail-Server und
fragt ab ob ungelesene eMails im Posteingang sind. Je nach Wert wird ein M oder ein N 
an der seriellen Port gesendet.

<h2>Vorraussetzungen</h2>
Installierte Python Module<br />
pySerial Library<br />
Universal Feed Parser Modul<br />
Cronjob


<h2>Projekte dazu im FluuuxTechBlog</h2>
<a target="_blank" href="http://fluuux.de/2012/09/wir-basteln-uns-einen-arduino-gmail-notifier/">Wir basteln uns einen Arduino gMail notifier</a>
