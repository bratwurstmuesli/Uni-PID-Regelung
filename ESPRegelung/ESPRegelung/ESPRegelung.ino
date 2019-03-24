/*
 Name:		ESPRegelung.ino
 Created:	1/22/2019 2:48:12 PM
 Author:	Heiko
*/
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

void setup() {
	//Serial########################################################
	Serial.begin(115200);
	Serial.println("TEST");

	//I2C###########################################################
	WireONE.begin(SDA1, SCL1, 400000); // SDA pin 21, SCL pin 22 TTGO TQ

	//PID###########################################################
	myPID.SetOutputLimits(0, 255);
	myPID.SetMode(AUTOMATIC);
}

void loop() {

	//Current#######################################################
	currentsense();

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
		setrpm();

	}

	//Alle10ms######################################################
	static unsigned long previousMillis1 = 0;
	currentMillis = millis();
	const long interval1 = 10;
	if (currentMillis - previousMillis1 >= interval1) {
		previousMillis1 = currentMillis;

		String print = "2000,0," + (String)(int)Setpoint + ',' + (String)(int)Output + ',' + (String)rpm;
		Serial.print(print);
		//String print2 = +',' + (String)(currentRA.getAverage() * 1000) );
		//Serial.print(print2);
		Serial.println();
	}
}
