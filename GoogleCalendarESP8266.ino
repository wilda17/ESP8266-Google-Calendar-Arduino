/*  Google Calendar Integration ESP8266
 *  Created by Daniel Willi, 2016
 *
 *  Based on the WifiClientSecure example by
 *  Ivan Grokhotkov
 *
 */

#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"

//Network credentials
const char* ssid = "ssid"; //replace with you ssid
const char* password = "password"; //replace with your password
//Google Script ID
const char *GScriptId = "gscriptid"; //replace with you gscript id

//Connection Settings
const char* host = "script.google.com";
const char* googleRedirHost = "script.googleusercontent.com";
const int httpsPort = 443;

//Fetch Google Calendar events
String url = String("/macros/s/") + GScriptId + "/exec";
//Delimiter for Events
const String delimeter = "|";

//Define port order: 0 is Port 16, 1 is Port 5
int portOrder[] = {16, 5, 4, 0, 2, 14, 12, 13, 15};
bool activePins[(int)( sizeof(portOrder) / sizeof(portOrder[0]))];

//#define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#ifdef DEBUG    //Macros are usually in all capital letters.
#define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
#define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
#define DPRINT(...)     //now defines a blank line
#define DPRINTLN(...)   //now defines a blank line
#endif

void setup() {
  Serial.begin(9600);
  Serial.flush();
  connectToWifi();
  setPortsAsOutput();
}

void loop() {
  fetchDataFromGoogle();
  setActivePins();
  delay(10000);
}

//Turn active pins from array on
void setActivePins() {
  for (int i = 0; i < (int)( sizeof(activePins) / sizeof(activePins[0])); i++) {
    //Turn active pins on
    if (activePins[i]) {
      digitalWrite(portOrder[i], HIGH);

      DPRINT("ON : ");
      DPRINT(i);
      DPRINT(" = ");
      DPRINT(portOrder[i]);
      DPRINT("\n");
    }
    //Turn not active pins off
    else {
      digitalWrite(portOrder[i], LOW);

      DPRINT("OFF: ");
      DPRINT(i);
      DPRINT(" = ");
      DPRINT(portOrder[i]);
      DPRINT("\n");
    }

    //After turning the pin on or off set it to off
    activePins[i] = false;
  }
  DPRINTLN("============");
}
//Set all pins from array portOrder as output
void setPortsAsOutput() {
  for (int i = 0; i < (int)( sizeof(portOrder) / sizeof(portOrder[0])); i++) {
    pinMode(portOrder[i], OUTPUT);
  }
}
//Connect to wifi
void connectToWifi() {

  DPRINTLN();
  DPRINT("Connecting to wifi: ");
  DPRINTLN(ssid);
  // flush() is needed to print the above (connecting...) message reliably,
  // in case the wireless connection doesn't go through
  Serial.flush();


  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DPRINT(".");
  }

  DPRINTLN("");
  DPRINTLN("WiFi connected");
  DPRINTLN("IP address: ");
  DPRINTLN(WiFi.localIP());

  // Use HTTPSRedirect class to create TLS connection
  HTTPSRedirect client(httpsPort);

  DPRINT("Connecting to ");
  DPRINTLN(host);

  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client.connect(host, httpsPort);
    if (retval == 1) {
      flag = true;
      break;
    }
    else
      DPRINTLN("Connection failed. Retrying...");
  }

  Serial.flush();
  if (!flag) {
    DPRINT("Could not connect to server: ");
    DPRINTLN(host);
    DPRINTLN("Exiting...");
    return;
  }
}
//Get calendar entries from google
void fetchDataFromGoogle() {
  HTTPSRedirect client(httpsPort);
  if (!client.connected())
    client.connect(host, httpsPort);

  String data = client.getData(url, host, googleRedirHost);

  bool p = false;
  for (int i = 0; i < data.length(); i++) {
    if ((String)data[i] == delimeter)
      p = !p;

    if (p && (String)data[i] != delimeter) {
      if (isdigit(data[i])) {
        activePins[data[i] - '0'] = true;
      }
    }
  }
}
