/*
 Name:		ESPRegelung.ino
 Created:	1/22/2019 2:48:12 PM
 Author:	Heiko
*/



//TODO
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

//median filter
#include <MedianFilter.h>
MedianFilter test(15, 0);

//Websocket#####################################################
#include "Task0.h"
#include "index.h"
WebSocketsServer webSocket = WebSocketsServer(81);


//I2C###########################################################
#include <Wire.h>
#define SDA1 21
#define SCL1 22
TwoWire WireONE = TwoWire(0);
unsigned long rpm;

boolean newsend;
boolean newrec;

long newValuetime;
//PID###########################################################
#include <PID_v1.h>
double Setpoint = 1000;
double Input, Output;	//Define Variables we'll be connecting to
double Kp = 1.30, Ki = 2.10, Kd = 0.10; //Specify the links and initial tuning parameters
long SampleTime = 100;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
boolean changeflag = false;

//RunningAverage###############################################
#include "RunningAverage.h"
RunningAverage InputRA(2);
RunningAverage InputNice(15);
RunningAverage CurrentRA(2000);

//speedpot######################################################
const byte speedpotpin = 33;

//Stromstärke-Messung###########################################
//MAX471
const byte strompin = 33;
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
	Input = (double)InputRA.getAverage();
	//Input = (double)rpm;
	myPID.Compute();
	newsend = true;
}

//TODO
void currentsense() {
	int currentint = analogRead(strompin);
	//Serial.println(currentint);
	//current = ((currentint / 4096) * 3.3) * 1000;
	CurrentRA.addValue(currentint);
	//currentRA.addValue(current);
	Serial.println(currentint);
	//Serial.print("Volt: ");
	//Serial.println(current);
}

//FULLY-WORKING
void sendI2C() {
	WireONE.beginTransmission(8); // transmit to device #8
	WireONE.write((int)Output);   // sends
	WireONE.endTransmission();    // stop transmitting
}

//FULLY WORKING
//receive I2C
//wert wird rpm
//zeit zwischen aktualisierung eines neuen Wertes
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

	unsigned long rpmnew = bigNum;
	static unsigned long rpmoldnewvalue;
	if (rpm != rpmnew) {
		newValuetime = micros() - rpmoldnewvalue;
		SampleTime = newValuetime / 1000;
		myPID.SetSampleTime(SampleTime);
		rpmoldnewvalue = micros();
		//Serial.println(newValuetime);


		test.in(rpmnew);
		if (rpmnew > test.out() * 1.2) {
			rpm = rpmnew;
			InputRA.addValue(test.out());
		}
		else {
			InputRA.addValue(rpmnew);
			rpm = rpmnew;
		}
		InputNice.addValue(InputRA.getAverage());


		newrec = true;

	}

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
				datain[i - 1] = (char)payload[i];
			}
			char delimiter[] = ",";
			char* ptr;

			// initialisieren und ersten Abschnitt erstellen
			ptr = strtok(datain, delimiter);

			int b = 1;
			double x = atof(ptr);
			Kp = x;
			b++;
			while (ptr != NULL) {
				//printf("Abschnitt gefunden: %s\n", ptr);
				//// naechsten Abschnitt erstellen
				ptr = strtok(NULL, delimiter);
				if (b == 2) {
					double y = atof(ptr);
					Ki = y;
				}
				else if (b == 3) {
					double z = atof(ptr);
					Kd = z;
				}
				else if (b == 4) {
					int w = atoi(ptr);
					Setpoint = (double)w;
				}
				b++;
			}
			myPID.SetTunings(Kp, Ki, Kd);
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
		String stringOne = String('D') + String(',') + (String)Kp + String(',') + (String)Ki + String(',') + (String)Kd + String(',') + (String)(int)Setpoint;
		//Serial.print("this is sent to websocket: ");
		//Serial.println(stringOne);
		char str[stringOne.length() + 1];
		stringOne.toCharArray(str, stringOne.length() + 1);
		webSocket.broadcastTXT(str, sizeof(str));
		//Serial.println("test2");
		//Serial.println(str);
		changeflag = false;
	}

	String stringTwo = String('R') + String(',') + (String)(int)InputNice.getAverage() + String(',') + (String)(int)Output;
	//Serial.println(stringTwo);
	char str1[stringTwo.length() + 1];

	stringTwo.toCharArray(str1, stringTwo.length() + 1);
	//Serial.println(str1);
	webSocket.broadcastTXT(str1, sizeof(str1));

}

void checkStall() {
	//write value to array and check when value < 500 ob gleich bleibt
	//wenn ja gib mal gas, wenn nicht dann ists so weit runtergeregelt
}

void setup() {
	//Serial########################################################
	Serial.begin(115200);
	Serial.println("TEST");

	//PID###########################################################
	myPID.SetOutputLimits(0, 255);
	myPID.SetMode(AUTOMATIC);
	myPID.SetSampleTime(SampleTime);
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

	//I2C###########################################################
	WireONE.begin(SDA1, SCL1, 400000); // SDA pin 21, SCL pin 22 TTGO TQ
}

void loop() {
	//Websocket#####################################################
	webSocket.loop();

	//Websites######################################################
	server.handleClient();

	//Current#######################################################
	//currentsense();

	//PID-Regelung##################################################
	//static unsigned long rpmOld;
	receiveI2C();
	if (newrec == true) {
		//if (rpm != rpmOld) {
		pidregel();
		sendI2C();
		newrec = false;
		//currentsense();

		String print = "2000,0," + (String)(int)Setpoint + ',' + (String)(int)Output + ',' + (String)(int)InputRA.getAverage() + ',' + (String)(int)InputNice.getAverage() + ',' + (String)(int)CurrentRA.getAverage();
		Serial.println(print);
	}

	//}
	//static double Outputold;
	//if (newsend == true) {
	//	if (Outputold != Output) {
	//		Outputold = Output;

	//	}
	//}


	//Alle1000ms####################################################
	static unsigned long previousMillis = 0;
	unsigned long currentMillis = millis();
	const long interval = 10;
	if (currentMillis - previousMillis >= interval) {
		previousMillis = currentMillis;

	}

	//Alle100ms#####################################################
	static unsigned long previousMillis2 = 0;
	currentMillis = millis();
	const long interval2 = 100;
	if (currentMillis - previousMillis2 >= interval2) {
		previousMillis2 = currentMillis;
		SendSocket();

	}

	//Alle10MICROSEKUNDE######################################################
	static unsigned long previousMillis1 = 0;
	currentMillis = micros();
	const long interval1 = 30;
	if (currentMillis - previousMillis1 >= interval1) {
		previousMillis1 = currentMillis;
		currentsense();

		//
	}
}
