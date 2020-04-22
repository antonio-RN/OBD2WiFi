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
char dataSent[7];
int dataReceived[6] = {0, 0, 0, 0, 0};
float finalData = 0.0;

void writeELMread(const char* PID="WS", uint16_t timeOUT = 5000) {
	if (client.connected())
	{
		strncat(dataSent, "AT", 3); 
		strncat(dataSent, PID, 5);
		client.print(dataSent);
		client.print('\r');
		unsigned long t0 = millis();
		unsigned long t1;
		Serial.print("Enviado a ELM: ");
		Serial.println(dataSent);
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
			t1 = millis();
			if (t1-t0>=timeOUT) {
				Serial.println("No se ha recibido el carácter > del ELM");
				break;
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
		strncat(dataSent, PID, 7);
		client.print(dataSent);
		client.print('\r');
		unsigned long t0 = millis();
		unsigned long t1;
		Serial.print("Enviado a OBD: ");
		Serial.println(dataSent);
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
			t1 = millis();
			if (t1-t0>=timeOUT) {
				Serial.println("No se ha recibido el carácter > del ELM");
				break;
			}
		}
	}
	else
	{
		Serial.println("Cliente desconectado, no se puede enviar el comando ELM");
	}
}

void writeOBDsave(const char* PID="", uint8_t totalSize = 1, factor = 1.0, offset = 0, uint16_t timeOUT = 500) {
	if (client.connected())
	{
		strncat(dataSent, PID, 7);
		for (int i=0; i < 5; i++) {
			dataReceived[i] = 0;	
		}
		finalData = 0.0;
		client.print(dataSent);
		client.print('\r');
		unsigned long t0 = millis();
		unsigned long t1;
		Serial.print("Enviado a OBD: ");
		Serial.println(dataSent);
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
				if (line[0] == '4') {
					for (int i = 0; i < totalSize; i++) {
						char HEXtemp[3] = "00"; //pointer?
						line.substring(2+i*2, 4+i*2).toCharArray(HEXtemp, 3);
						dataReceived[i] = (int) strtol(HEXtemp, 0, 16);
					}
				transfer();
				Serial.print("Los valores recibidos son: ");
					for (i = 0; i < totalSize; i++){
						Serial.print(dataReceived[i]);	
					}
				Serial.println();
				Serial.print("El valor resultante es: ");
				Serial.println(finalData);
				}
			}
			t1 = millis();
			if (t1-t0>=timeOUT) {
				Serial.println("No se ha recibido el carácter > del ELM");
				break;
			}
		}
	}
	else
	{
		Serial.println("Cliente desconectado, no se puede enviar el comando ELM");
	}
}

void transfer() {
	if (dataReceived[1] == '0') {
		finalData = factor*dataReceived[0]-offset:
	}
	else {
		finalData = factor*(256*dataReceived[0]+dataReceived[1])-offset;
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
	writeOBDsave("0142", 2, 0.001, 0);
	delay(500);
	writeOBDread("010C");
	delay(500);
	writeOBDsave("010C", 2, 0.25, 0);
	delay(500);
	writeOBDread("015C");
	delay(500);
	writeOBDsave("015C", 1, 0.0, 40);
	delay(500);
}
