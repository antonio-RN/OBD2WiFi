#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <string.h> 

#ifndef STASSID
#define STASSID "WiFi_OBDII"
#define STAPSK ""
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
const char* host = "192.168.0.10";
const uint16_t port = 35000;

void writeELMread(char* PID="WS", uint16_t delSp = 500) {
	if (client.connected())
	{
		String sentToElm = "AT"+PID+'\r';
		client.print(sentToElm);
		Serial.println("Enviado a ELM: "+sentToElm);
		delay(delSp);
		while (client.available()){
			String line = client.readStringUntil('\r');
			Serial.println("Recibido de ELM: "+line);
		}
	}
	else
	{
		Serial.println("Cliente desconectado, no se puede enviar el comando ELM");
	}
}

void writeOBDread(char* PID="", uint16_t delSp = 500) {
	if (client.connected())
	{
		String sentToOBD = PID+'\r';
		client.print(sentToOBD);
		Serial.println("Enviado a OBD: "+sentToOBD);
		delay(delSp);
		while (client.available()){
			String line = client.readStringUntil('\r');
			Serial.println("Recibido de OBD: "+line);
		}
	}
	else
	{
		Serial.println("Cliente desconectado, no se puede enviar el comando OBD");
	}
}

String writeOBDsave(char* PID="", uint8_t bytesEx = 1, uint16_t delSp = 500) {
	if (client.connected())
	{
		String sentToOBD = PID+'\r';
		client.print(sentToOBD);
		Serial.println("Enviado a OBD: "+sentToOBD);
		delay(delSp);
		String line = "";
		while (client.available()){
			line += client.readStringUntil('\r');
		}
		Serial.println("Recibido de OBD: "+line);
		String resp = line.substr(4,bytesEx*2);
		return resp;
	}
	else
	{
		Serial.println("Cliente desconectado, no se puede enviar el comando OBD");
		return "ERROR";
	}
}

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
    delay(2000);
    Serial.println("Reintentando conexión al servidor");
  }
  Serial.print("Conexión establecida al servidor");
  delay(1000);
  
  writeELMread("Z");
  writeELMread("M0");
  Serial.println("Detectando protocolo OBD...");
  writeELMread("TP0",4000);
  writeELMread("DP");
  writeELMread("DPN");
  writeELMread("RV");
  delay(1000);

  // while (client.available() == 0){
	  // delay(1000);
	  // Serial.print(".");
  // }
  // unsigned long timeout = millis();
  // while (client.available() == 0)
  // {
    // if (millis() - timeout > 5000)
    // {
      // Serial.println("Timeout sin respuesta, terminando conexión");
      // client.stop();
      // return;
    // }
  // }
  
  // while (client.available())
  // {
    // String line = client.readStringUntil('\r');
    // Serial.println(line);
  // }
  // Serial.println();
  // Serial.println("Cerrando conexión con servidor OBD2");
  // if (client.connected())
  // {
    // client.stop();
  // }
}

void loop() {
	writeOBDread("0142",2,3000);
}