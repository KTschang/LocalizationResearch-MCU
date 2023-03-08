/*
  Instructions:
  - Update WiFi SSID and password as necessary.
  - Flash the sketch to a ESP8266 board
*/

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#ifndef STASSID
//#define STASSID "WSN-2.4GHz"
//#define STAPSK  "silentquail118"
#define STASSID "Galaxy S10f7b5"
#define STAPSK  "kfyf6731"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;
const char* apssid = "ESPap";
const char* appassword = "thereisnospoon";
char hostString[16] = {0};
static bool wait = true;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\r\nsetup()");

  sprintf(hostString, "ESP_%06X", ESP.getChipId());
  Serial.print("Hostname: ");
  Serial.println(hostString);
  WiFi.hostname(hostString);
  //WiFi.softAP(apssid, appassword);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin(hostString)) {
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");

  Serial.println("loop() next");

  WiFi.softAP(apssid, appassword);
}

void loop() {
  static bool searchForAndroid = true;
  String host;
  uint16_t port;  

  while(searchForAndroid) {
    // put your main code here, to run repeatedly:
    MDNS.update();

    Serial.println("Sending mDNS query");
    int n = MDNS.queryService("NSDTest", "tcp", 5000); 
    Serial.println("mDNS query done");
    if (n == 0) {
      Serial.println("no services found");
    } else {
      searchForAndroid = false;
      Serial.print(n);
      Serial.println(" service(s) found");
      for (int i = 0; i < n; ++i) {
        // Print details for each service found
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(MDNS.hostname(i));
        Serial.print(" (");
        Serial.print(MDNS.IP(i));
        host = MDNS.IP(i).toString();
        Serial.print(":");
        Serial.print(MDNS.port(i));
        port = MDNS.port(i);
        Serial.println(")");
      }
    }
  }
  
  

  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    delay(5000);
    searchForAndroid = true;
    return;
  }

  // This will send a string to the server
  Serial.println("sending data to server");
  if (client.connected()) {
    client.println("hello from ESP8266");
  }

  // wait for data to be available
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      delay(60000);
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  Serial.println("receiving from remote server");
  // not testing 'client.connected()' since we do not need to send data here
  while (client.available()) {
    char ch = static_cast<char>(client.read());
    Serial.print(ch);
  }

  while (client.connected()) {
    Serial.println("Sending ping");
    char outstr[50];
    sprintf(outstr, "Ping from: %d.%d.%d.%d at %d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3], millis());
    Serial.println(outstr);
    client.print(outstr);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    delay(2000);
  }

  // Close the connection
  //Serial.println();
  //Serial.println("closing connection");
  //client.stop();
  //searchForAndroid = true;

  if (wait) {
    delay(300000); // execute once every 5 minutes, don't flood remote service
  }
}
