/*
 Name:		ESPRegelung.ino
 Created:	1/22/2019 2:48:12 PM
 Author:	Heiko
*/
//##############################################################
#include <PID_v1.h>
double Setpoint, Input, Output;	//Define Variables we'll be connecting to
double Kp = 10, Ki = 10, Kd = 0; //Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
//##############################################################
#include "RunningAverage.h"
RunningAverage myRA(100);
RunningAverage currentRA(100);
RunningAverage currentRA2(1000);
//##############################################################
//H-Bridge L9110
//TWO Digital Outputs
const int speepin1 = 12;
const int speedpin2 = 14;
// setting PWM properties
const int freq = 40000;
const int ledChannel = 0;
const int ledChannel1 = 1;
const int resolution = 8;
//speedpot######################################################
const int speedpotpin = 34;
//Stromstärke-Messung###########################################
//MAX471
const int strompin = 25;
float current;
//##############################################################
//Lichtschranke
const int dataIN = 33; //IR sensor INPUT
volatile unsigned long prevmillis; // To store time
volatile unsigned long duration; // To store time difference
volatile unsigned long refresh; // To store time for refresh of reading
volatile int rpm; // RPM value
volatile int rpmold;
boolean currentstate; // Current state of IR input scan
boolean prevstate; // State of IR sensor in previous scan

boolean newvalueflag; //neuer wert für rpm
//##############################################################

//WORKING
void speedcontrol(int pwm, char a) {
	if (a == 'f') {
		ledcWrite(ledChannel, pwm);
		digitalWrite(speedpin2, LOW);
	}
	else if (a == 'b') {
		digitalWrite(speedpin2, LOW);
		ledcWrite(ledChannel1, pwm);
	}
	else {
		ledcWrite(ledChannel, 0);
		ledcWrite(ledChannel1, 0);
	}
	//Serial.print("PWM: ");
	//Serial.println(pwm);
}

//TODO
void currentsense() {
	int currentint = analogRead(strompin);
	//runningAverage(currentint);
	current = (currentint * 3.3) / 4096;
	currentRA.addValue(current);
	currentRA2.addValue(current);
	//Serial.print("int: ");
	//Serial.println(currentint);
	//Serial.print("Volt: ");
	//Serial.println(current);
}

//WORKING
void rpmmeasure() { // RPM Measurement
	currentstate = digitalRead(dataIN); // Read IR sensor state
	if (prevstate != currentstate) // If there is change in input
	{
		if (currentstate == HIGH) // If input only changes from LOW to HIGH
		{
			duration = (micros() - prevmillis); // Time difference between revolution in microsecond
			rpm = (60000000 / duration); // rpm = (1/ time millis)*1000*1000*60;
			if (rpm > 2500 || rpm > rpmold + 700) {
				rpm = rpmold;
			}
			else {
				rpmold = rpm;
			}
			myRA.addValue(rpm);
			prevmillis = micros(); // store time for nect revolution calculation
			newvalueflag = true;
		}
	}
	prevstate = currentstate; // store this scan (prev scan) data for next scan
	if (micros() - prevmillis > 500000) {
		rpm = 0;
	}
}

//WORKING
void setspeed() {
	int analogvalue = analogRead(speedpotpin);
	int mapvalue = map(analogvalue, 0, 4096, 0, 255);
	speedcontrol(mapvalue, 'f');
	//Serial.println(analogvalue);
}

void setup() {
	//##############################################################
	Serial.begin(115200);
	Serial.println("TEST");
	//##############################################################
	pinMode(dataIN, INPUT);
	prevmillis = 0;
	prevstate = LOW;
	myPID.SetMode(AUTOMATIC);
	//##############################################################
	//PWM einbauen
	//https://randomnerdtutorials.com/esp32-pwm-arduino-ide/
	//https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/ledc.html
	ledcSetup(ledChannel, freq, resolution);
	ledcAttachPin(speepin1, ledChannel);
	pinMode(speedpin2, OUTPUT);
	//##############################################################
}

void loop() {
	//##############################################################
	//RPM MEASURE
	rpmmeasure();
	//##############################################################
	//PID
	if (newvalueflag == true) {
		newvalueflag = false;
		//if (isnan(myRA.getAverage())) {
		//	Input = 0;
		//}
		//else {
		//	Input = (int)myRA.getAverage();
		//	myRA.clear();
		//}
		Input = rpm;
		myPID.Compute();
		speedcontrol((int)Output, 'f');

	}

	//##############################################################
	//CURRENT
	currentsense();
	//##############################################################
	//Alle 1000ms
	static unsigned long previousMillis = 0;
	unsigned long currentMillis = millis();
	const long interval = 1000;
	if (currentMillis - previousMillis >= interval) {
		previousMillis = currentMillis;



	}
	//##############################################################
	//alle 100ms
	static unsigned long previousMillis2 = 0;
	unsigned long currentMillis2 = millis();
	const long interval2 = 100;
	if (currentMillis2 - previousMillis2 >= interval2) {
		previousMillis2 = currentMillis2;

		int analogvalue = analogRead(speedpotpin);
		Setpoint = map(analogvalue, 0, 4096, 300, 1900);
	}
	//##############################################################
	//alle 10ms
	static unsigned long previousMillis1 = 0;
	unsigned long currentMillis1 = millis();
	const long interval1 = 0;
	if (currentMillis1 - previousMillis1 >= interval1) {
		previousMillis1 = currentMillis1;

		String print = "2000,0," + (String)(int)Setpoint + ',' + (String)(int)Output + ',' + (String)rpm;
		Serial.print(print);
		//String print2 = +',' + (String)(currentRA.getAverage() * 1000) + ',' + (String)(currentRA2.getAverage() * 1000);
		//Serial.print(print2);
		Serial.println();

			//Serial.print("Setpoint: ");
			//Serial.println(Setpoint);
			//Serial.print("rpm: ");
			//Serial.println((int)myRA.getAverage());
			//Serial.print("duration: ");
			//Serial.println(duration);
			//Serial.println();

	}
	//##############################################################
}
