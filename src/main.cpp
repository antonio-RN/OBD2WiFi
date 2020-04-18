#include <Arduino.h>
#include <ESP8266WiFi.h>

#define STASSID "WiFi_OBDII"
#define STAPSK ""
#define STAIP "192.168.0.10"
#define STAPORT 35000

const char* ssid = STASSID;
const char* password = STAPSK;
const char* host = STAIP;
const uint16_t port = STAPORT;
WiFiClient client;

void writeELMread(const char* PID="WS", uint16_t timeOUT = 5000) {
	if (client.connected())
	{
		char sentToElm[2+sizeof(PID)];
		strcat(sentToElm, "AT");
		strcat(sentToElm, PID);
		client.print(sentToElm);
		client.print('\r');
		unsigned long t0 = millis();
		unsigned long t1;
		Serial.print("Enviado a ELM: ");
		Serial.println(sentToElm);
		while (!client.available()){
			t1 = millis();
			if (t1-t0>=timeOUT) {
				Serial.println("No se ha recibido respuesta de ELM");
				break;
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

void writeOBDread(const char* PID="", uint16_t timeOUT = 500) {
	if (client.connected())
	{
		String sentToOBD = PID+'\r';
		client.print(sentToOBD);
		unsigned long t0 = millis();
		unsigned long t1;
		Serial.println("Enviado a OBD: "+sentToOBD);
		while (!client.available()){
			t1 = millis();
			if (t1-t0>=timeOUT) {
				Serial.println("No se ha recibido respuesta de OBD");
				break;
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

u_int* writeOBDsave(const char* PID="", uint16_t timeOUT = 500) {
	u_int* HEXreturn;
	if (client.connected())
	{
		String sentToOBD = PID+'\r';
		client.print(sentToOBD);
		unsigned long t0 = millis();
		unsigned long t1;
		Serial.println("Enviado a OBD: "+sentToOBD);
		while (!client.available()){
			t1 = millis();
			if (t1-t0>=timeOUT) {
				Serial.println("No se ha recibido respuesta de OBD");
				break;
			}
		}
		while (client.available()){
			if (client.find('>'))
			{
				String line = client.readStringUntil('\r');
				Serial.println("Recibido de OBD: "+line);
				String HEXstring = line.substring(4);
				for (int i = 0; i < HEXstring.length()/2; i++)
				{
					char HEXsubstring[2];
					HEXstring.substring(i*2,i*2+2).toCharArray(HEXsubstring,2);
					HEXreturn += atoi(HEXsubstring);
				}
			}
		}
	}
	else
	{
		Serial.println("Cliente desconectado, no se puede enviar el comando OBD");
	}
	return HEXreturn;
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