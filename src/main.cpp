#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "SPI.h"
#include <TFT_eSPI.h>


#define STASSID "WiFi_OBDII"
#define STAPSK ""
#define STAIP "192.168.0.10"
#define STAPORT 35000

#define TFT_CS_1   PIN_D8
#define TFT_CS_2   PIN_D9

const char* ssid = STASSID;
const char* password = STAPSK;
const char* host = STAIP;
const uint16_t port = STAPORT;
const uint8_t timeWait = 50; //prueba y error
WiFiClient client;
uint8_t actualMode = 0;
uint8_t desiredMode = 1; // 1 = normal, 2 = sport; 0 = bienvenida, nunca en desiredMode. Falta input para cambiar de modo
bool tOilLow = 1;
TFT_eSPI display = TFT_eSPI();
char dataSent[7];
uint8_t dataReceived[6] = {0, 0, 0, 0, 0};
float finalData = 0.0;

float vBat = 0.0;
int8_t tAmb = 0;
float tank = 0.0;
uint8_t vel = 0;
float RPM = 0.0; 
uint8_t gear = 0;
uint8_t tWat = 0;
uint8_t tOil = 0;
float tLoad = 0.0;
uint16_t pFuel = 0.0;


void transfer(float factor, int16_t offset) {
	if (dataReceived[1] == '0') {
		finalData = factor*dataReceived[0]-offset;
	}
	else {
		finalData = factor*(256*dataReceived[0]+dataReceived[1])-offset;
	}
}

uint8_t HEXtoInt (char str[]){
	return strtol(str,0,16);
}

int8_t floatToInt8(float floatIn){
	return floatIn;
}

uint8_t floatToUint8(float floatIn){
	return floatIn;
}

uint16_t floatToUint16(float floatIn){
	return floatIn;
}

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

void writeOBDread(const char* PID="", uint16_t timeOUT = 500) { //writeOBDsave hace lo mismo y más, eliminar cuadno funcione
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

void writeOBDsave(const char* PID="", uint8_t totalSize = 1, float factor = 1.0, int16_t offset = 0, uint16_t timeOUT = 500) {
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
						dataReceived[i] = HEXtoInt(HEXtemp);
					}
				transfer(factor, offset);
				Serial.print("Los valores recibidos son: ");
					for (int i = 0; i < totalSize; i++){
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

void exeMode(uint8_t desiredMode = 1) {
	if (desiredMode != actualMode){
		digitalWrite(TFT_CS_1, LOW);
		digitalWrite(TFT_CS_2, LOW);
		display.fillScreen(TFT_BLACK);
		digitalWrite(TFT_CS_1, HIGH);
		digitalWrite(TFT_CS_2, HIGH);
		actualMode = desiredMode;
	}
	int16_t tempWidth = 0;
	if (desiredMode == 0) { //bienvenida: voltaje batería, temperatura ambiente, tanque restante (o batería, a implementar después)

		writeOBDsave("015B", 2, 0.001, 0); //voltaje batería. Display 1. Datum centrado, x100y100 fuente 6
		vBat = finalData;
		digitalWrite(TFT_CS_1, LOW);
		display.setTextDatum(4);
		tempWidth = display.textWidth("99.9", 6);
		display.setTextPadding(tempWidth);
		display.drawFloat(vBat, 1, 100, 100, 6);
		digitalWrite(TFT_CS_1, HIGH);
		delay(timeWait);

		writeOBDsave("0146", 1, 1.0, 40); //temperatura ambiente. Display 1. Datum extremo abajo derecho, x240y240 fuente 4
		tAmb = floatToInt8(finalData);
		digitalWrite(TFT_CS_1, LOW);
		display.setTextDatum(8);
		tempWidth = display.textWidth("-99", 4);
		display.setTextPadding(tempWidth);
		display.drawNumber(tAmb, 240, 240, 4);
		digitalWrite(TFT_CS_1, HIGH);
		delay(timeWait);

		writeOBDsave("022F", 1, 0.392, 0); //tanque restante. Display 2. Datum centrado, x140y100
		tank = finalData;
		digitalWrite(TFT_CS_2, LOW);
		display.setTextDatum(4);
		tempWidth = display.textWidth("999", 6);
		display.setTextPadding(tempWidth);
		display.drawFloat(tank, 0, 140, 100, 6);
		digitalWrite(TFT_CS_2, HIGH);
		delay(timeWait);
	}

	else if (desiredMode == 1) { //normal: voltaje batería, temperatura ambiente, tanque restante (o batería, a implementar después)
				//velocidad actual, RPM actual, marcha engranada - falta probar esta útlima, no info -(posible susituto?)

		writeOBDsave("015B", 2, 0.001, 0); //voltaje batería. Display 1. Datum arriba derecha, x240y0 fuente 2
		vBat = finalData;
		digitalWrite(TFT_CS_1, LOW);
		display.setTextDatum(2);
		tempWidth = display.textWidth("99.9", 2);
		display.setTextPadding(tempWidth);
		display.drawFloat(vBat, 1, 240, 0, 2);
		digitalWrite(TFT_CS_1, HIGH);
		delay(timeWait);

		writeOBDsave("0146", 1, 1.0, 40); //temperatura ambiente. Display 1. Datum arriba izquierda, x10y0 fuente 2
		tAmb = floatToInt8(finalData);
		digitalWrite(TFT_CS_1, LOW);
		display.setTextDatum(0);
		tempWidth = display.textWidth("-99", 2);
		display.setTextPadding(tempWidth);
		display.drawNumber(tAmb, 10, 0, 2);
		digitalWrite(TFT_CS_1, HIGH);
		delay(timeWait);

		writeOBDsave("022F", 1, 0.392, 0); //tanque restante. Display 2. Datum arriba derecha, x230y0 fuente 2
		tank = finalData;
		digitalWrite(TFT_CS_2, LOW);
		display.setTextDatum(2);
		tempWidth = display.textWidth("999", 2);
		display.setTextPadding(tempWidth);
		display.drawFloat(tank, 0, 230, 0, 2);
		digitalWrite(TFT_CS_2, HIGH);
		delay(timeWait);

		writeOBDsave("010C", 2, 0.25, 0); //RPM actual. Display 2. Datum centrado, x120y120 fuente 6
		RPM = finalData;
		digitalWrite(TFT_CS_2, LOW);
		display.setTextDatum(4);
		tempWidth = display.textWidth("9999", 6);
		display.setTextPadding(tempWidth);
		display.drawFloat(RPM, 0, 120, 120, 6);
		digitalWrite(TFT_CS_2, HIGH);
		delay(timeWait);

		writeOBDsave("010D", 1, 1.0, 0); //velocidad actual. Display 1. Datum centrado, x100y120 fuente 6
		vel = floatToUint8(finalData);
		digitalWrite(TFT_CS_1, LOW);
		display.setTextDatum(4);
		tempWidth = display.textWidth("999", 6);
		display.setTextPadding(tempWidth);
		display.drawNumber(vel, 100, 120, 6);
		digitalWrite(TFT_CS_1, HIGH);
		delay(timeWait);

		writeOBDsave("01A4", 1, 1.0, 0); //marcha engranada (suposición). Display 1. Datum abajo derecha, x240y240 fuente 6
		gear = floatToUint8(finalData);
		digitalWrite(TFT_CS_1, LOW);
		display.setTextDatum(8);
		tempWidth = display.textWidth("9", 6);
		display.setTextPadding(tempWidth);
		display.drawNumber(gear, 240, 240, 6);
		digitalWrite(TFT_CS_1, HIGH);
		delay(timeWait);
	}
	else if (desiredMode ==2) { //sport: tanque restante (o batería, a implementar después)
				//marcha engranada - falta probar esta útlima, no info -(posible susituto?), temperatura refri
				//temperatura aceite, par motor (decidir cuál), presión fuel

		writeOBDsave("015C", 1, 1.0, 40); //temperatura aceite
		tOil = floatToUint8(finalData);
		if (tOil<85) {
			if(!tOilLow) {
				tOilLow = !tOilLow;
				digitalWrite(TFT_CS_1, LOW);
				display.fillScreen(TFT_BLACK);				
			}
			digitalWrite(TFT_CS_1, LOW);
			display.setTextDatum(4);
			tempWidth = display.textWidth("999", 6);
			display.setTextPadding(tempWidth);
			display.setTextColor(TFT_RED);
			display.drawNumber(tOil, 120, 170, 6);
			display.setTextColor(TFT_BLACK);
			digitalWrite(TFT_CS_1, HIGH);
			delay(timeWait);
			
			writeOBDsave("0105", 1, 1.0, 40); //temperatura refri motor
			tWat = floatToUint8(finalData);
			digitalWrite(TFT_CS_1, LOW);
			display.setTextDatum(4);
			tempWidth = display.textWidth("999", 4);
			display.setTextPadding(tempWidth);
			display.drawNumber(tWat, 120, 50, 4);
			digitalWrite(TFT_CS_1, HIGH);
			delay(timeWait);
		}
		else {
			if (tOilLow) {
				tOilLow = !tOilLow;
				digitalWrite(TFT_CS_1, LOW);
				display.fillScreen(TFT_BLACK);
				
			}
			digitalWrite(TFT_CS_1, LOW);
			display.setTextDatum(4);
			tempWidth = display.textWidth("999", 4);
			display.setTextPadding(tempWidth);
			display.drawNumber(tOil, 70, 170, 4);
			digitalWrite(TFT_CS_1, HIGH);
			delay(timeWait);
			
			writeOBDsave("022F", 1, 0.392, 0); //tanque restante
			tank = finalData;
			digitalWrite(TFT_CS_1, LOW);
			display.setTextDatum(4);
			tempWidth = display.textWidth("999", 4);
			display.setTextPadding(tempWidth);
			display.drawFloat(tank, 0, 70, 70, 4);
			digitalWrite(TFT_CS_1, HIGH);
			delay(timeWait);
			
			writeOBDsave("0105", 1, 1.0, 40); //temperatura refri motor
			tWat = floatToUint8(finalData);
			digitalWrite(TFT_CS_1, LOW);
			display.setTextDatum(4);
			tempWidth = display.textWidth("999", 4);
			display.setTextPadding(tempWidth);
			display.drawNumber(tWat, 170, 170, 4);
			digitalWrite(TFT_CS_1, HIGH);
			delay(timeWait);
			
			writeOBDsave("010A", 1, 3.0, 0); //presión fuel
			pFuel = floatToUint16(finalData);
			digitalWrite(TFT_CS_1, LOW);
			display.setTextDatum(4);
			tempWidth = display.textWidth("999", 4);
			display.setTextPadding(tempWidth);
			display.drawFloat(pFuel, 170, 70, 4);
			digitalWrite(TFT_CS_1, HIGH);
			delay(timeWait);
		}
			
		writeOBDsave("010C", 2, 0.25, 0); //RPM actual
		RPM = finalData;
		digitalWrite(TFT_CS_2, LOW);
		display.setTextDatum(5);
		tempWidth = display.textWidth("9999", 4);
		display.setTextPadding(tempWidth);
		display.drawFloat(RPM, 0, 180, 40, 4);
		digitalWrite(TFT_CS_2, HIGH);
		delay(timeWait);

		writeOBDsave("010D", 1, 1.0, 0); //velocidad actual
		vel = floatToUint8(finalData);
		digitalWrite(TFT_CS_2, LOW);
		display.setTextDatum(5);
		tempWidth = display.textWidth("999", 4);
		display.setTextPadding(tempWidth);
		display.drawNumber(vel, 180, 120, 4);
		digitalWrite(TFT_CS_2, HIGH);
		delay(timeWait);

		writeOBDsave("01A4", 1, 1.0, 0); //marcha engranada (suposición)
		gear = floatToUint8(finalData);
		digitalWrite(TFT_CS_2, LOW);
		display.setTextDatum(6);
		tempWidth = display.textWidth("9", 6);
		display.setTextPadding(tempWidth);
		display.drawFloat(gear, 0, 240, 6);
		digitalWrite(TFT_CS_2, HIGH);
		delay(timeWait);

		writeOBDsave("0143", 2, 0.392, 0); //par motor (absoluto) -provisional-
		tLoad = finalData;
		digitalWrite(TFT_CS_2, LOW);
		display.setTextDatum(5);
		tempWidth = display.textWidth("999", 4);
		display.setTextPadding(tempWidth);
		display.drawFloat(tLoad, 180, 200, 4);
		digitalWrite(TFT_CS_2, HIGH);
		delay(timeWait);
	}
}

void setup() {
	pinMode(TFT_CS_1, OUTPUT);
 	pinMode(TFT_CS_2, OUTPUT);
	
	Serial.begin(115200);
	Serial.println();
	Serial.println("#OBD2WiFi#");

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid,password);
	
	digitalWrite(TFT_CS_1, LOW);
	digitalWrite(TFT_CS_2, LOW);
	display.init();
	display.setRotation(0);
	display.fillScreen(TFT_BLACK);
	display.setTextSize(1);
	display.setTextDatum(0);
	display.setTextFont(4);
	display.setTextPadding(0);
	display.setTextColor(TFT_WHITE, TFT_BLACK);
	display.setCursor(0, 0);
	digitalWrite(TFT_CS_1, HIGH);
	digitalWrite(TFT_CS_2, HIGH);
	
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
  
	// pruebas iniciales, borrar lo máximo posible una vez funcione todo OK
	writeELMread("Z");
	writeELMread("E0");
	writeELMread("M0");
	Serial.println("Detectando protocolo OBD...");
	writeELMread("TPA1",10000);
	writeELMread("DP");
	writeELMread("DPN");
	writeELMread("RV");
	
	unsigned long t0 = millis()-1;
	unsigned long t1 = millis();
	
	while (t1-t0 < 5000) {
		exeMode(0);
		t1 = millis();
	}
}

void loop() {
	// modo prueba, para ver los valores de respuesta y debug. Borrar en versión final
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
	
	//modo normal, comentar en pruebas
	exeMode(desiredMode);	
}
