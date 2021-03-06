#include <WiFi.h>
#include <Wire.h>
#include "DHT.h"

// using dht 11
#define DHTTYPE DHT11
// Defines pin number to which the sensor is connected
uint8_t DHTPin = 4;

unsigned long previousMillis = 0;
unsigned long interval = 30000;

// creates a dht object
DHT dht(DHTPin, DHTTYPE);

float Temperature;
float Humidity;

void send_event(const char *event);

const char *ssid = "AndroidAPAA86";
const char *password = "wiuw1034";

const char *host = "maker.ifttt.com";
// Enter the privte key that was copied from IFTTT Webhooks
const char *privateKey = "cPj1CIZhXwOo302JQTTz02";

// 80 is the default port for HTTP
WiFiServer server(80);

String header;

void initWiFi()
{
  WiFi.mode(WIFI_STA);
  // to connect to the network
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  // to check if the connection was established or not
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
}

void setup()
{
  // opens serial port, sets data rate to 115200 bps
  Serial.begin(115200);
  // configure DHTPin to behave either as an input
  pinMode(DHTPin, INPUT);
  // Initialize the DHT sensor
  dht.begin();

  initWiFi();

  Serial.println("");

  Serial.println("Successfully connected to WiFi.");

  Serial.println("IP address of ESP32 is : ");
  // generates IP address
  Serial.println(WiFi.localIP());

  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  // tells the server to begin listening for incoming connections
  server.begin();
  Serial.println("Server started");
}

void loop()
{
  Temperature = dht.readTemperature();
  Humidity = dht.readHumidity();

  WiFiClient client = server.available();

  if (client)
  {
    Serial.println("Web Client connected ");
    String request = client.readStringUntil('\r');
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");

    client.println("Connection: close"); // the connection will be closed after completion of the response
    client.println("Refresh: 10");       // update the page after 10 sec

    client.println();

    client.println("<!DOCTYPE HTML>");

    client.println("<html>");

    client.println("<style>html { font-family: Cairo; display: block; margin: 0px auto; text-align: center;color: #333333; background-color: ##f3ffee;}");

    client.println("body{margin-top: 50px;}");

    client.println("h1 {margin: 50px auto 30px; font-size: 50px;  text-align: center;}");

    client.println(".side_adjust{display: inline-block;vertical-align: middle;position: relative;}");

    client.println(".text1{font-weight: 180; padding-left: 15px; font-size: 50px; width: 170px; text-align: left; color: #3498db;}");

    client.println(".data1{font-weight: 180; padding-left: 70px; font-size: 50px;color: #3498db;}");

    client.println(".text2{font-weight: 180;padding-left: 15px; font-size: 50px; width: 170px; text-align: left; color: #ff6600;}");

    client.println(".data2{font-weight: 180; padding-left: 120px; font-size: 50px;color: #ff6600;}");

    client.println(".data{padding: 10px;}");

    client.println("</style>");

    client.println("</head>");

    client.println("<body>");

    client.println("<div id=\"webpage\">");

    client.println("<h1>ESP32 Interfacing with DHT11</h1>");

    client.println("<div class=\"data\">");

    client.println("<div class=\"side_adjust text1\">Humidity:</div>");

    client.println("<div class=\"side_adjust data1\">");

    client.print(Humidity);

    client.println("<div class=\"side_adjust text1\">%</div>");

    client.println("</div>");

    client.println("<div class=\"data\">");

    client.println("<div class=\"side_adjust text2\">Temperature:</div>");

    client.println("<div class=\"side_adjust data2\">");

    client.print(Temperature);

    client.println("<div class=\"side_adjust text2\">*C</div>");

    client.println("</div>");

    client.println("</div>");

    client.println("</body>");

    client.println("</html>");

    delay(4000);

    if (Temperature >= 20)
    {

      send_event("temp_event");
    }
  }

  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval))
  {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
}

void send_event(const char *event)
{
  Serial.print("Connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;

  const int httpPort = 80;
  if (!client.connect(host, httpPort))
  {

    Serial.println("Connection failed");

    return;
  }

  // We now create a URI for the request
  String url = "/trigger/";
  url += event;
  url += "/with/key/";
  url += privateKey;

  Serial.print("Requesting URL: ");

  Serial.println(url);

  // This will send the request to the server

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +

               "Host: " + host + "\r\n" +

               "Connection: close\r\n\r\n");

  while (client.connected())
  {
    if (client.available())
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    else
    {
      // No data yet, wait a bit
      delay(50);
    };
  }

  Serial.println();

  Serial.println("closing connection");

  client.stop();
}
