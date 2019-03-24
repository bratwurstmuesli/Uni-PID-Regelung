/*
 Name:		ESPRegelung.ino
 Created:	1/22/2019 2:48:12 PM
 Author:	Heiko
*/

//PID###########################################################
#include <PID_v1.h>


double Setpoint, Input, Output;	//Define Variables we'll be connecting to
double Kp = 2, Ki = 2, Kd = 0; //Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

//RunningAverage###############################################
#include "RunningAverage.h"
RunningAverage myRA(100);
RunningAverage currentRA(100);

//H-Brücke######################################################
//L9110
//Two Digital Outputs
const byte speepin1 = 12;
const byte speedpin2 = 14;
//setting PWM properties
const int freq = 20000;
const int ledChannel = 0;
const int ledChannel1 = 1;
const int resolution = 8;

//speedpot######################################################
const byte speedpotpin = 34;

//Stromstärke-Messung###########################################
//MAX471
const byte strompin = 25;
float current;

//Lichtschranke#################################################
const byte dataIN = 33; //IR sensor INPUT
volatile unsigned long prevmillis; // To store time
volatile unsigned long duration; // To store time difference
volatile unsigned long refresh; // To store time for refresh of reading
volatile int rpm; // RPM value
volatile int rpmold;
boolean currentstate; // Current state of IR input scan
boolean prevstate; // State of IR sensor in previous scan
boolean newvalueflag; //neuer wert für rpm

//FULLY_WORKING
void speedcontrol(int pwm, char a) {
	if (a == 'f') {
		ledcWrite(ledChannel, pwm);
		digitalWrite(speedpin2, LOW);
	}
	else if (a == 'b') {

	}
	else {
		ledcWrite(ledChannel, 0);
		digitalWrite(speedpin2, LOW);
	}
}

//FULLY_WORKING
void setrpm() {
	int analogvalue = analogRead(speedpotpin);
	//Setpoint = map(analogvalue, 0, 4096, 0, 2000);
	Setpoint = map(analogvalue, 0, 4096, 0, 255);
}

//OKAISH-WORKING
//wenn groeßer 500ms ist dann wird 0 oder RPM groeßer 2500 oder RPM ploetzlich um 700 steigt dann wird alter wert genutzt
void rpmmeasure() { // RPM Measurement
	currentstate = digitalRead(dataIN); // Read IR sensor state
	if (prevstate != currentstate) { // If there is change in input
		if (currentstate == HIGH) { // If input only changes from LOW to HIGH
			duration = (micros() - prevmillis); // Time difference between revolution in microsecond
			rpm = (60000000 / duration); // rpm = (1/ time millis)*1000*1000*60;
			//if (rpm > 2500 || rpm > rpmold + 700) {
			//	rpm = rpmold;
			//}
			//else {
			//	rpmold = rpm;
			//}
			//
			prevmillis = micros(); // store time for nect revolution calculation
			newvalueflag = true;
		}
	}
	prevstate = currentstate; // store this scan (prev scan) data for next scan
	if (micros() - prevmillis > 500000) {
		rpm = 0;
	}
	myRA.addValue(rpm);
}

//OKAISH-WORKING
void pidregel() {
	if (newvalueflag == true) {
		newvalueflag = false;
		if (isnan(myRA.getAverage())) {
			Input = rpm;
		}
		else {
			Input = (int)myRA.getAverage();
		}

		//Input = rpm;

	}
	myPID.Compute();

	Output = Setpoint;
	speedcontrol((int)Output, 'f');
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

void setup() {
	//Serial########################################################
	Serial.begin(115200);
	Serial.println("TEST");

	//PID###########################################################
	pinMode(dataIN, INPUT);
	myPID.SetOutputLimits(0, 255);
	prevmillis = 0;
	prevstate = LOW;
	myPID.SetMode(AUTOMATIC);

	//PWM###########################################################
	//https://randomnerdtutorials.com/esp32-pwm-arduino-ide/
	//https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/ledc.html
	ledcSetup(ledChannel, freq, resolution);
	ledcAttachPin(speepin1, ledChannel);
	pinMode(speedpin2, OUTPUT);
}

void loop() {
	//RPM-Measure###################################################
	rpmmeasure();

	//Current#######################################################
	currentsense();

	//PID-Regelung##################################################
	pidregel();

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
