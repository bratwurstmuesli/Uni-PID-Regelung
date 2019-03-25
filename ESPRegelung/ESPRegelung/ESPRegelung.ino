/*
 Name:		ESPRegelung.ino
 Created:	1/22/2019 2:48:12 PM
 Author:	Heiko
*/
//Webserver#####################################################
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WebSocketsServer.h>

const char* ssid = "\xe2\x9c\x8c\xef\xb8\x8f\xf0\x9f\x98\x81\xe2\x9c\x8c\xef\xb8\x8f";
const char* password = "123mannheim#1";

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

String Websocket_page = "<!DOCTYPE html><html><style>input[type=\"text\"]{width: 90%; height: 3vh;}input[type=\"button\"]{width: 9%; height: 3.6vh;}.rxd{height: 90vh;}textarea{width: 99%; height: 100%; resize: none;}</style><script>var Socket;function start(){Socket=new WebSocket('ws://' + window.location.hostname + ':81/'); Socket.onmessage=function(evt){document.getElementById(\"rxConsole\").value +=evt.data;}}function enterpressed(){Socket.send(document.getElementById(\"txbuff\").value); document.getElementById(\"txbuff\").value=\"\";}</script><body onload=\"javascript:start();\"> <div><input class=\"txd\" type=\"text\" id=\"txbuff\" onkeydown=\"if(event.keyCode==13) enterpressed();\"><input class=\"txd\" type=\"button\" onclick=\"enterpressed();\" value=\"Send\" > </div><br><div class=\"rxd\"> <textarea id=\"rxConsole\" readonly></textarea> </div></body></html>";

//include HTML SEITE
#include "index.h"
#include "Task0.h"
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

void handledata() {
	String data = "";

	data = "Kp: ";
	data += (String)Kp;
	data += " ";

	data += "Ki: ";
	data += (String)Ki;
	data += " ";

	data += "Kd: ";
	data += (String)Kd;
	data += " ";

	data += "Setpoint: ";
	data += (String)Setpoint;
	data += " ";

	data += "Input: ";
	data += (String)Input;
	data += " ";

	data += "Output: ";
	data += (String)Output;
	data += " ";

	server.send(200, "text/html", data);
}

void handlenotfound() {
	//server.send(404, "text/plain", "404: Not found");
	server.sendHeader("Location", "/", true);   //Redirect to our html web page
	server.send(302, "text/plane", "");
}
//|| server.hasArg("delay1") || server.hasArg("delay2") || server.hasArg("delay3")
void handlemainweb() {
	if (server.hasArg("delay0")) {
		handleSubmit();
		Serial.println("test1");
	}
	else {
		String page = MAIN_page;
		server.send(200, "text/html", page);
	}
	Serial.println("test4");
}
//<INPUT type = "text" style = "width:100px" name = "delay1">
//<INPUT type = "text" style = "width:100px" name = "delay2">
//<INPUT type = "text" style = "width:100px" name = "delay3">

void returnFail(String msg)
{
	server.sendHeader("Connection", "close");
	server.sendHeader("Access-Control-Allow-Origin", "*");
	server.send(500, "text/plain", msg + "\r\n");
}

void handleSubmit()
{
	Serial.println("test2");
	String string0;
	String string1;
	String string2;
	String string3;

	if (server.hasArg("delay0")) {
		string0 = server.arg("delay0");
		Kp = string0.toInt();
		Serial.println("test3");
	}
	if (server.hasArg("delay1")) {
		string0 = server.arg("delay1");
		Ki = string1.toInt();
	}
	if (server.hasArg("delay2")) {
		string0 = server.arg("delay2");
		Kd = string2.toInt();
	}
	if (server.hasArg("delay3")) {
		string0 = server.arg("delay3");
		Setpoint = string3.toInt();
	}

	Serial.println("new: " + (String)(int)Kp + (String)(int)Ki + (String)(int)Kd + (String)(int)Setpoint);
	Serial.println("done");
	//String page = MAIN_page;
	//server.send(200, "text/html", page);
}

void handleKp() {
	String data = (String)Kp;
	server.send(200, "text/html", data);
}

void handleKi() {
	String data = (String)Ki;
	server.send(200, "text/html", data);
}

void handleKd() {
	String data = (String)Kd;
	server.send(200, "text/html", data);
}

void handleSetpoint() {
	String data = (String)Setpoint;
	server.send(200, "text/html", data);
}

void handleInput() {
	String data = (String)Input;
	server.send(200, "text/html", data);
}

void handleOutput() {
	String data = (String)Output;
	server.send(200, "text/html", data);
}

void debug() {
	String data = "debug Webpage";
	server.send(200, "text/html", data);
}

//FULLY_WORKING
void setrpm() {
	int analogvalue = analogRead(speedpotpin);
	Setpoint = map(analogvalue, 0, 4095, 0, 200000);
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
	if (type == WStype_TEXT) {
		for (int i = 0; i < length; i++) Serial.print((char)payload[i]);
		Serial.println();
	}
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

	if (MDNS.begin("esp32")) {
		Serial.print("mDNS responder started: http://");
		Serial.print("esp32");
		Serial.println(".local");
	}

	//Pages
	//Mainseite
	server.on("/", handlemainweb);

	//datenkomplett
	server.on("/data.txt", handledata);

	//einzelne Daten
	server.on("/Kp", handleKp);
	server.on("/Ki", handleKi);
	server.on("/Kd", handleKd);
	server.on("/Setpoint", handleSetpoint);
	server.on("/Input", handleInput);
	server.on("/Output", handleOutput);

	server.on("/debug", debug);

	server.onNotFound(handlenotfound);
	server.begin();

	//websocket
	webSocket.begin();
	webSocket.onEvent(webSocketEvent);
	server.on("/websocket", []() {
		server.send(200, "text/html", Websocket_page);
	});
}

void loop() {
	webSocket.loop();
	server.handleClient();

	if (Serial.available() > 0) {
		char c[] = { (char)Serial.read() };
		webSocket.broadcastTXT(c, sizeof(c));
	}

	//Current#######################################################
	//currentsense();

	//PID-Regelung##################################################
	//receiveI2C();
	//pidregel();
	//sendI2C();

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
		setrpm();

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
	}
}
