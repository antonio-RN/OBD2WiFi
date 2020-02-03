#include <Arduino.h>
#include <ESP8266WiFi.h>

#ifndef STASSID
#define STASSID "AP-OBD"
#define STAPSK "muysegura"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
const char* host = "192.168.0.10";
const uint16_t port = 35000;

void setup() {
  Serial.begin(115200);
  Serial.println("#OBD2WiFi#");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
  Serial.print("Conectando a módulo OBD2...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado con IP ");
  Serial.println(WiFi.localIP());
  Serial.print("Conectando al servidor OBD2 ");
  Serial.print(host);
  Serial.print(":");
  Serial.println(port);
  WiFiClient client;
  while (!client.connect(host,port))
  {
    Serial.println("Error conectando al servidor OBD2");
    delay(5000);
    Serial.println("Reintentando conexión al servidor");
  }
  Serial.println("Conexión establecida al servidor");
  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    if (millis() - timeout > 5000)
    {
      Serial.println("Timeout sin respuesta, terminando conexión");
      client.stop();
      return;
    }
  }
  while (client.available())
  {
    char ch = static_cast<char>(client.read());
    Serial.print(ch);
  }
  Serial.println();
  Serial.println("Cerrando conexión con servidor OBD2");
  if (client.connected())
  {
    client.stop();
  }
  
}

void loop() {
}