/*
MoWeSta Beispielcode für Solo Beginner
*/

#include <WiFi.h>                                                           //Einbinden der Bibliothek des ESP32, speziell hier für das WiFi

#include <Adafruit_BME280.h>                                                //Einbinden der Bibliothek des Sensors
#include <Adafruit_Sensor.h>

#define SEALEVELPRESSURE_HPA (1013.25)                                      // Standardreferenzwert zur Ermittlung des Drucks

Adafruit_BME280 bme;                                                        // Für die Kommunikation mittels I2C
WiFiServer server(80);                                                      // Portdefinition für den Server, der später auf einer Webseite die Daten anzeigt
String textfeld;                                                            // Speicher für die Daten der Webseitenansicht

void setup() {                                                              // Starten der einzelnen Komponenten
  Serial.begin(115200);                                                     // Für die serielle Kommunikation

  if (!bme.begin(0x76)) {                                                   // Überprüfung ob der Sensor angeschlossen ist, alternative Adresse ist 0x75
    Serial.println("BME280 nicht gefunden!");
    while (1);
  }

  Serial.println("Verbindung zum Netzwerk wird aufgebaut");                 // Einbinden ins Heimnetzwerk
  WiFi.begin("Netzwerkname eintragen", " Netzwerkpassword eintragen");      // Name und Password des Netzwerks
  while (WiFi.status() != WL_CONNECTED) {                                   // Einfache Statusanzeige
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("Erfolgreich verbunden - IP Addresse:");                   // IP Adresse zum Verbinden anzeigen
  Serial.println(WiFi.localIP());
  server.begin();                                                           // Server starten
}

void loop(){                                                                // Hier wird das eigentliche Programm in einer Schleife (loop) ausgeführt
  WiFiClient client = server.available();                                   // Überprüft ob eine Anfrage vorhanden ist
  
  if (client) {                                                             // Falls ein Client sich verbindet
    Serial.println("Neuer Client:");                                        // Für die Anzeige im Seriellen Monitor das sich wer verbunden hat
    String position = "";                                                   // Platzhalter für eingehende Nachrichten
    
    while (client.connected()) {                                            // Schleife währendessen ein Client verbunden ist
      if (client.available()) {                                             // Wenn es was zu lesen gibt, sammeln und in den Serial schreiben
        char symbol = client.read();                                             
        Serial.write(symbol);                                                    
        textfeld += symbol;
        if (symbol == '\n') {                                               // Wenn eine neue Zeile kommt                                                  
                                                                            
          if (position.length() == 0) {                                     // Hier ist die HTTP-Antwort beendet und nun kann was geschickt werden
            
            client.println("HTTP/1.1 200 OK");                              // Einfacher HTTP-textfeld
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            client.println("<!DOCTYPE html><html>");                         // Ab hier wird die HTML-Seite gezeigt                                             
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                        
            client.println("<style>body { text-align: center; font-family: Arial;}");             // Eine schöne Tabelle erstellen
            client.println("table { border-collapse: collapse; margin-left:auto; margin-right:auto; }");
            client.println("th { padding: 12px; background-color: #00695c; color: white; }");
            client.println("tr { border: 1px solid #dddddd; padding: 12px; }");
            client.println("td { border: none; padding: 12px; }");
            
            client.println("</style></head><body><h1>Umweltmessung</h1>");                        // Text über der Tabelle
            client.println("<table><tr><th>Messgr&ouml;&szlig;e </th><th>Messwert</th></tr>");    // Namen der Tabellen Eintrage
            
            client.println("<tr><td>Temperatur</td><td>");                  // Eintrag Temperatur
            client.println(bme.readTemperature());
            client.println(" *C</td></tr>");     
            
            client.println("<tr><td>Druck</td><td>");                       // Eintrag Druck - Messung durch 100.0F für hPa
            client.println(bme.readPressure() / 100.0F);
            client.println(" hPa</td></tr>");

            client.println("<tr><td>Luftfeuchtigkeit</td><td>");            // Eintrag Luftfeuchtigkeit
            client.println(bme.readHumidity());
            client.println(" %</td></tr>"); 
            client.println("</body></html>");

            break;                                                          // Schleife beenden
          } else {                                                          // Ansonsten wird position wieder zurückgesetzt / freigemacht
            position = "";
          }
        } else if (symbol != '\r') {                                        // Ansonsten wenn alles andere als ein \r (Carriage-return)kommt, an position anhängen
          position += symbol;                                                 
        }
      }
    }
    textfeld = "";                                                          // Variable textfeld wird wieder zurückgesetzt / freigemacht
    
    client.stop();                                                          // Verbindung wird beendet
    Serial.println("Verbindung zum Client Beendet.");
  }
}
