/*
 Name:		ESPRegelung.ino
 Created:	1/22/2019 2:48:12 PM
 Author:	Heiko
*/



//TODO
//PID values with double
//draw graph
//implement current measurement
//cascade control


//Webserver#####################################################
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WebSocketsServer.h>
const char* ssid = "\xe2\x9c\x8c\xef\xb8\x8f\xf0\x9f\x98\x81\xe2\x9c\x8c\xef\xb8\x8f";
const char* password = "123mannheim#1";
WebServer server(80);

//Websocket#####################################################
WebSocketsServer webSocket = WebSocketsServer(81);
#include "Task0.h"
#include "index.h"

//I2C###########################################################
#include <Wire.h>
#define SDA1 21
#define SCL1 22
TwoWire WireONE = TwoWire(0);
unsigned long rpm;

//PID###########################################################
#include <PID_v1.h>
double Setpoint, Input, Output;	//Define Variables we'll be connecting to
double Kp = 2, Ki = 2, Kd = 0; //Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
boolean changeflag = false;

//RunningAverage###############################################
#include "RunningAverage.h"
RunningAverage myRA(100);
RunningAverage currentRA(100);

//speedpot######################################################
const byte speedpotpin = 33;

//Stromstärke-Messung###########################################
//MAX471
const byte strompin = 25;
float current;


void handlenotfound() {
	//server.send(404, "text/plain", "404: Not found");
	server.sendHeader("Location", "/websocket", true);   //Redirect to our html web page
	server.send(302, "text/plane", "");
}

void returnFail(String msg)
{
	server.sendHeader("Connection", "close");
	server.sendHeader("Access-Control-Allow-Origin", "*");
	server.send(500, "text/plain", msg + "\r\n");
}

//FULLY_WORKING
void setrpm() {
	int analogvalue = analogRead(speedpotpin);
	int Setpointint = map(analogvalue, 0, 4095, 0, 100);
	static int Setpointintold;
	if (Setpointint != Setpointintold) {
		Setpointintold = Setpointint;
		Setpoint = (double)(Setpointint * 2000);
		changeflag = true;
		Serial.println("rpm set with poti.");
	}
}


//OKAISH-WORKING
void pidregel() {
	Input = (double)rpm;
	myPID.Compute();
}

//TODO
void currentsense() {
	int currentint = analogRead(strompin);
	current = (currentint * 3.3) / 4096;
	currentRA.addValue(current);
	//Serial.println(currentint);
	//Serial.print("Volt: ");
	//Serial.println(current);
}

//FULLY-WORKING
void sendI2C() {
	WireONE.beginTransmission(8); // transmit to device #8
	WireONE.write((int)Output);   // sends
	WireONE.endTransmission();    // stop transmitting
}

void receiveI2C() {
	int32_t bigNum;
	byte a, b, c, d;

	WireONE.requestFrom(8, 4);

	a = WireONE.read();
	b = WireONE.read();
	c = WireONE.read();
	d = WireONE.read();

	bigNum = a;
	bigNum = (bigNum << 8) | b;
	bigNum = (bigNum << 8) | c;
	bigNum = (bigNum << 8) | d;

	rpm = (unsigned)(long)bigNum;
	//Serial.println(bigNum);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
	switch (type) {
	case WStype_DISCONNECTED:
		Serial.println("Websocket disconnected");
		break;

	case WStype_CONNECTED:
		Serial.println("Websocket connected.");
		break;

	case WStype_TEXT:
		char datain[100];
		if ((char)payload[0] == '#') {
			for (int i = 1; i < length; i++) {
				datain[i-1] = (char)payload[i];
			}
			char delimiter[] = ",";
			char *ptr;

			// initialisieren und ersten Abschnitt erstellen
			ptr = strtok(datain, delimiter);

			int b = 1;
			int x = atoi(ptr);
			Kp = (double)x;
			b++;
			while (ptr != NULL) {
				//printf("Abschnitt gefunden: %s\n", ptr);
				//// naechsten Abschnitt erstellen
				ptr = strtok(NULL, delimiter);
				if (b == 2) {
					int y = atoi(ptr);
					Ki = (double)y;
				}
				else if (b == 3) {
					int z = atoi(ptr);
					Kd = (double)z;
				}
				else if (b == 4) {
					int w = atoi(ptr);
					Setpoint = (double)w;
				}
				b++;
			}
			changeflag = true;
		}
		else {
			Serial.print("sent from websocket: ");
			for (int i = 0; i < length; i++) {
				Serial.print((char)payload[i]);
			}
			Serial.println();
		}
		break;
	}
}

void SendSocket() {
	//Serial.println("test1");
		if (changeflag == true) {
			String stringOne = String('D') + String(',') + (String)(int)Kp + String(',') + (String)(int)Ki + String(',') + (String)(int)Kd + String(',') + (String)(int)Setpoint;
			//Serial.print("this is sent to websocket: ");
			//Serial.println(stringOne);
			char str[stringOne.length() + 1];
			stringOne.toCharArray(str, stringOne.length() + 1);
			webSocket.broadcastTXT(str, sizeof(str));
			//Serial.println("test2");
			//Serial.println(str);
			changeflag = false;
		}
		
		String stringTwo = String('R') + String(',') + (String)(int)Input + String(',') + (String)(int)Output;
		//Serial.println(stringTwo);
		char str1[stringTwo.length() + 1];

		stringTwo.toCharArray(str1, stringTwo.length() + 1);
		//Serial.println(str1);
		webSocket.broadcastTXT(str1, sizeof(str1));
	
}

void setup() {
	//Serial########################################################
	Serial.begin(115200);
	Serial.println("TEST");

	//I2C###########################################################
	WireONE.begin(SDA1, SCL1, 400000); // SDA pin 21, SCL pin 22 TTGO TQ

	//PID###########################################################
	myPID.SetOutputLimits(0, 255);
	myPID.SetMode(AUTOMATIC);

	//Wifi##########################################################
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	//mDNS##########################################################
	if (MDNS.begin("esp32")) {
		Serial.print("mDNS responder started: http://");
		Serial.print("esp32");
		Serial.println(".local");
	}

	//Websites######################################################
	//Pages
	//Mainseite
	server.onNotFound(handlenotfound);
	server.begin();

	//Websocket#####################################################
	webSocket.begin();
	webSocket.onEvent(webSocketEvent);
	server.on("/websocket", []() {
		server.send(200, "text/html", Websocket_page);
	});
}

void loop() {
	//Websocket#####################################################
	webSocket.loop();

	//Websites######################################################
	server.handleClient();

	//Current#######################################################
	//currentsense();

	//PID-Regelung##################################################
	receiveI2C();
	pidregel();
	sendI2C();

	//Alle1000ms####################################################
	static unsigned long previousMillis = 0;
	unsigned long currentMillis = millis();
	const long interval = 1000;
	if (currentMillis - previousMillis >= interval) {
		previousMillis = currentMillis;

	}

	//Alle100ms#####################################################
	static unsigned long previousMillis2 = 0;
	currentMillis = millis();
	const long interval2 = 100;
	if (currentMillis - previousMillis2 >= interval2) {
		previousMillis2 = currentMillis;
		//setrpm();
		SendSocket();

	}

	//Alle10ms######################################################
	static unsigned long previousMillis1 = 0;
	currentMillis = millis();
	const long interval1 = 10;
	if (currentMillis - previousMillis1 >= interval1) {
		previousMillis1 = currentMillis;

		//String print = "2000,0," + (String)(int)Setpoint + ',' + (String)(int)Output + ',' + (String)rpm;
		//Serial.print(print);
		//String print2 = +',' + (String)(currentRA.getAverage() * 1000) );
		//Serial.print(print2);
		//Serial.println();

		//print to websocket
		//char c[print.length() + 1];
		//print.toCharArray(c, print.length()+1);
		//webSocket.broadcastTXT(c, sizeof(c));
	}
}
