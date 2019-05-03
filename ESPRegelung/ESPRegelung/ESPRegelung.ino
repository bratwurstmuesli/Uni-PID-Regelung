/*
 Name:		ESPRegelung.ino
 Created:	1/22/2019 2:48:12 PM
 Author:	Heiko
*/

//Project start 22.01.2019
//Code after CodeCleanup Day 02.05.2019

//Webserver#####################################################
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WebSocketsServer.h>

//name meines wlans daheim
const char* ssid = "\xe2\x9c\x8c\xef\xb8\x8f\xf0\x9f\x98\x81\xe2\x9c\x8c\xef\xb8\x8f";
const char* password = "123mannheim#1";

//hier gewünschten hotspotnamen eingeben
const char* ssid1 = "DHBW Drehzahlregelung";
const char* password1 = "studienarbeit";

WebServer server(80);

//Websocket#####################################################
#include "Task0.h"
//#include "index.h" //alt. hier versuch mit Ajax anstatt Websocket. allerdings nicht zufriedenstellend
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

//regelung or steuerung
//PID OR PWM
//1 or 2
int switchmode = 1;
int PWM_wert; //alternative variable für webserver
boolean changepwm;
//FILTERING###############################################
//running average
#include "RunningAverage.h"
RunningAverage InputRA(2);
RunningAverage InputNice(15);
RunningAverage CurrentRA(100);
//median filter
#include <MedianFilter.h>
MedianFilter test(15, 0);

//speedpot######################################################
const byte speedpotpin = 33; // Not in use anymore
//used to set setpoint with poti -> not in use anymore because doesnt follow planned design

//Stromstärke-Messung###########################################
//MAX471
const byte strompin = 33;
const byte groundpin = 26;
RunningAverage CurrentRA2(100); //auch filter aber current ist extraproblem
float current;
int currentaveragefinal;


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

//not in use anymore
//void setrpm() {
//	int analogvalue = analogRead(speedpotpin);
//	int Setpointint = map(analogvalue, 0, 4095, 0, 100);
//	static int Setpointintold;
//	if (Setpointint != Setpointintold) {
//		Setpointintold = Setpointint;
//		Setpoint = (double)(Setpointint * 2000);
//		changeflag = true;
//		Serial.println("rpm set with poti.");
//	}
//}


void pidregel() {
	//Serial.println("anfang PIDREGEL");
	if (switchmode == 1) {
		Input = (double)InputRA.getAverage();
		//Input = (double)rpm;
		myPID.Compute();
	}
	newsend = true;
}


void currentsense() {
	//Serial.println("anfang currentsense");
	//int currentint = analogRead(strompin);
	//Serial.println(currentint);
	//current = ((currentint / 4096) * 3.3) * 1000;
	//CurrentRA.addValue(currentint);
	//currentRA.addValue(current);
	//Serial.println(currentint);
	//Serial.print("Volt: ");
	//Serial.println(current);
	int currentaverage = CurrentRA.getAverage();
	if (currentaverage <= 10000) {
		float currentaveragefloat = ((currentaverage / 4096.00) * 3.30) * 1000.00;
		currentaverage = (int)currentaveragefloat;
		CurrentRA2.addValue(currentaverage);
		static unsigned long previousMillis5 = 0;
		unsigned long currentMillis5 = millis();
		const long interval5 = 10;
		if (currentMillis5 - previousMillis5 >= interval5) {
			previousMillis5 = currentMillis5;
			currentaveragefinal = CurrentRA2.getAverage();
		}
	}
}

//I2C senden
void sendI2C() {
	//Serial.println("anfang sendI2C");
	WireONE.beginTransmission(8); // transmit to device #8
	if (switchmode == 1) {
		WireONE.write((int)Output);   // send PWM
	}
	else if (switchmode == 2) {
		WireONE.write(PWM_wert);
	}

	WireONE.endTransmission();    // stop transmitting
}

//FULLY WORKING
//receive I2C
//wert wird rpm
//zeit zwischen aktualisierung eines neuen Wertes
void receiveI2C() {
	//Serial.println("anfang I2C");
	int32_t bigNum;
	byte a, b, c, d;

	//funktionsweise: I2C kann keine 4 byte variable schicken
	//daher 4 bytes schicken und durch bitshifting eine 4 byte variable (int32_t = long) schaffen

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
		SampleTime = newValuetime / 1000; //hier neue SampleTime bilden um auf veränderete Drehzahl anzupassen (umrechnung us in ms)
		myPID.SetSampleTime(SampleTime); //Drehzahlwerte werden unteschiedlich schnell geliefert je nach Drehzahl -> ständiges Anpassen
		rpmoldnewvalue = micros();
		//Serial.println(newValuetime);

		//einkommende Werte filtern
		//https://de.mathworks.com/help/signal/ref/medfilt1.html
		//und große Werte werden auch herausgefiltert
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

//Websocket Werte von Website empfangen
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
	//Serial.println("anfang empfangesocket");
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
			myPID.SetTunings(Kp, Ki, Kd); //PID anpasssen
			changeflag = true;
		}
		else if ((char)payload[0] == 'A') {
			Serial.print("A ERKANNT");
			Serial.println("sent from websocket: ");
			for (int i = 0; i < length; i++) {
				Serial.print((char)payload[i]);
			}
			Serial.println();
			Serial.println("DEBUG");
			for (int i = 1; i < length; i++) {
				datain[i - 1] = (char)payload[i];
			}
			char delimiter[] = ",";
			char* ptr;

			// initialisieren und ersten Abschnitt erstellen
			ptr = strtok(datain, delimiter);
			int b = 1;
			int x = atoi(ptr);
			PWM_wert = x;
			Serial.println("neuer PWM: " + PWM_wert);
			changeflag = true;
			changepwm = true;

		}
		else if ((char)payload[0] == 'S') {
			Serial.println("S ERKANNT");
			Serial.println("sent from websocket: ");
			for (int i = 0; i < length; i++) {
				Serial.print((char)payload[i]);
			}
			Serial.println();
			Serial.println("DEBUG");
			for (int i = 1; i < length; i++) {
				datain[i - 1] = (char)payload[i];
			}
			char delimiter[] = ",";
			char* ptr;

			// initialisieren und ersten Abschnitt erstellen
			ptr = strtok(datain, delimiter);

			int b = 1;
			int x = atoi(ptr);
			switchmode = x;
			Serial.println("switchmode: " + switchmode);
		}
		else {
			Serial.println("sent from websocket: ");
			for (int i = 0; i < length; i++) {
				Serial.print((char)payload[i]);
			}
			Serial.println();
			Serial.println("DEBUG");
		}
		Socketaktualisieren();
		break;
	//case WStype_ERROR:
	//	Serial.print("ERROR");
	//case WStype_FRAGMENT_TEXT_START:
	//	Serial.print("ERROR");
	//case WStype_FRAGMENT_BIN_START:
	//	Serial.print("ERROR");
	//case WStype_FRAGMENT:
	//	Serial.print("ERROR");
	//case WStype_FRAGMENT_FIN:
	//	Serial.print("ERROR123");
	//	break;
	}
}

void Socketaktualisieren() {
	//Serial.println("Sockelaaktualisierung");
	if (changeflag == true) {
		if (switchmode == 1) {
			//Serial.println("String erstellen1");
			String stringOne = String('D') + String(',') + (String)Kp + String(',') + (String)Ki + String(',') + (String)Kd + String(',') + (String)(int)Setpoint;
			//Serial.print("this is sent to websocket: ");
			//Serial.println(stringOne);
			char str[stringOne.length() + 1];
			stringOne.toCharArray(str, stringOne.length() + 1);
			//Serial.println("String senden 1");
			webSocket.broadcastTXT(str, sizeof(str));
			//Serial.println("test2");
			//Serial.println(str);
			changeflag = false;
		}
		else if (switchmode == 2) {
			//Serial.println("String erstellen2");
			String stringDrei = String('P') + String(',') + (String)PWM_wert;
			char str3[stringDrei.length() + 1];
			stringDrei.toCharArray(str3, stringDrei.length() + 1);
			//Serial.println("String senden 2");
			webSocket.broadcastTXT(str3, sizeof(str3));
			changeflag = false;
		}
	}
}
//senden zum Websocket
void SendSocket() {
	//Serial.println("anfang sendsocket");
	//Serial.println("test1");

	int niceInputaver = (int)InputNice.getAverage();
	int input2send;
	if (niceInputaver > 10000) {
		input2send = 0;
	}
	else {
		input2send = niceInputaver;
	}
	//Serial.println("String erstellen3");
	String stringTwo = String('R') + String(',') + (String)input2send + String(',') + (String)(int)Output;
	//Serial.println(stringTwo);
	//Serial.println("string öffnen");
	char str1[stringTwo.length() + 1];
	//Serial.println("string tochararray");
	stringTwo.toCharArray(str1, stringTwo.length() + 1);
	//Serial.println(str1);
	//Serial.println("String senden 3");
	webSocket.broadcastTXT(str1, sizeof(str1));
	//Serial.println("string 3 gesendet");
}

void outputSerial() {
	//Serial.println("anfang serialoutput");
	if (currentaveragefinal < 10000) { //sonst liefert es -1 was als eine seeehr große Zahl erkannt wird weil nicht unsigned. daher nur ausgeben wenn denn der Wert passt.
		String print = "2000,0," + (String)(int)Setpoint + ',' + (String)(int)InputNice.getAverage();
		Serial.println(print);
		//alternativ:
		//String print = "2000,0," + (String)(int)Setpoint + ',' + (String)(int)Output + ',' + (String)(int)InputRA.getAverage() + ',' + (String)(int)InputNice.getAverage() + ',' + (String)(int)currentaveragefinal;
		//Serial.println(print);
	}
}
void anschieben() {
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
	//zum Wlan connecten
	//WiFi.mode(WIFI_STA);
	//WiFi.begin(ssid, password);
	//while (WiFi.status() != WL_CONNECTED) {
	//	delay(500);
	//	Serial.print(".");
	//}
	//neues Wlan aufmachen
	WiFi.softAP(ssid1, password1);
	//++++++++++++++++++++++++++++++++++
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

	//stromsensor
	pinMode(groundpin, OUTPUT);
	//digitalWrite(groundpin, LOW); //unter anderem ein Versuch mit kondensator zu entstören.. hat nicht geklappt
}

void loop() {
	//Serial.println("loopbegin");
	//Serial.setDebugOutput(true);
	//Websocket#####################################################
	webSocket.loop();

	//Websites######################################################
	server.handleClient();

	//um etwas von I2C zu empfangen muss der Master den Slave anfragen - Maser=ESP Slave=Arduino
	receiveI2C(); //so oft wie möglich werte empfangen

	//und dann nach neu empfangenen Werten ausführen:
	if (newrec == true && switchmode == 1) {//was passiert wenn neue werte erhalten wurden
		//if (rpm != rpmOld) {
		pidregel(); //zunächst neuen Stellwert berechnen
		sendI2C(); //dann zum arduino schicken
		//currentsense();
		outputSerial();
		newrec = false; //flag false setzen
	}
	if (switchmode == 2 && changepwm == true) {
		changepwm = false;
		sendI2C();
	}

	//Getimete Events hier:
	//Alle500ms####################################################
	static unsigned long previousMillis = 0;
	unsigned long currentMillis = micros();
	const long interval = 500;
	if (currentMillis - previousMillis >= interval) {
		previousMillis = currentMillis;

	}

	//Alle200ms#####################################################
	static unsigned long previousMillis2 = 0;
	currentMillis = millis();
	const long interval2 = 200;
	if (currentMillis - previousMillis2 >= interval2) {
		previousMillis2 = currentMillis;
		SendSocket(); //Socket in getimeten Event um Verbindung nicht zu überlasten
	}

	//Alle1000MICROSEKUNDEN##########################################
	static unsigned long previousMillis1 = 0;
	currentMillis = micros();
	const long interval1 = 1000;
	if (currentMillis - previousMillis1 >= interval1) {
		previousMillis1 = currentMillis;

	}
}
