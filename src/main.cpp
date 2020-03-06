#include <Arduino.h>
#include <ESP8266WiFi.h>

#define STASSID "WiFi_OBDII"
#define STAPSK ""


const char* ssid = STASSID;
const char* password = STAPSK;
const char* host = "192.168.0.10";
const uint16_t port = 35000;
WiFiClient client;

void writeELMread(String PID="WS", uint16_t timeOUT = 5000) {
	if (client.connected())
	{
		String sentToElm = "AT"+ PID +'\r';
		client.print(sentToElm);
		unsigned uint_16t t0 = millis();
		unsigned uint_16t t1;
		Serial.println("Enviado a ELM: "+sentToElm);
		while (!client.available()){
			t1 = millis();
			if (t1-t0>=timeOUT) {
				Serial.println("No se ha recibido respuesta de ELM");
				break
			}
		}
		while (client.available()){
			if (client.find('>'))
			{
			String line = client.readStringUntil('\r');
			Serial.println("Recibido de ELM: "+line);
			}
		}
	}
	else
	{
		Serial.println("Cliente desconectado, no se puede enviar el comando ELM");
	}
}

void writeOBDread(String PID="", uint16_t timeOUT = 500) {
	if (client.connected())
	{
		String sentToOBD = PID+'\r';
		client.print(sentToOBD);
		unsigned uint_16t t0 = millis();
		unsigned uint_16t t1;
		Serial.println("Enviado a OBD: "+sentToOBD);
		while (!client.available()){
			t1 = millis();
			if (t1-t0>=timeOUT) {
				Serial.println("No se ha recibido respuesta de OBD");
				break
			}
		}
		while (client.available()){
			if (client.find('>'))
			{
			String line = client.readStringUntil('\r');
			Serial.println("Recibido de OBD: "+line);
			}
		}
	}
	else
	{
		Serial.println("Cliente desconectado, no se puede enviar el comando OBD");
	}
}

void setup() {
	Serial.begin(115200);
	Serial.println();
	Serial.println("#OBD2WiFi#");

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid,password);

	Serial.print("Conectando a módulo OBD2...");
	while (WiFi.status() != WL_CONNECTED)
	{
    delay(200);
    Serial.print(".");
  }
	Serial.println();
	Serial.print("Conectado con IP ");
	Serial.println(WiFi.localIP());
	Serial.print("Conectando al servidor OBD2 ");
	Serial.print(host);
	Serial.print(":");
	Serial.println(port);

	while (!client.connect(host,port))
	{
		Serial.println("Error conectando al servidor OBD2");
		delay(2000);
		Serial.println("Reintentando conexión al servidor");
	}
	Serial.println("Conexión establecida al servidor");
	delay(1000);
  
	writeELMread("Z");
	writeELMread("E0");
	writeELMread("M0");
	Serial.println("Detectando protocolo OBD...");
	writeELMread("TPA1",10000);
	writeELMread("DP");
	writeELMread("DPN");
	writeELMread("RV");
}

void loop() {
	writeOBDread("0142");
	delay(500);
	writeOBDread("010C");
	delay(500);
	writeOBDread("015C");
	delay(500);
}
