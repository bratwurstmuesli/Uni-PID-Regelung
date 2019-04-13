#include <Wire.h>

//########################################################################
//Lichtschranke
const byte Lichtschrankepin = 2;
// variables used by the ISR
volatile long isrRevCount = 0; //wie oft interrupt bisher
volatile long timeold;
//RPM
volatile  long RPM = 0;
bool rotationNew = false;
//########################################################################
//SetSpeed
int setPWM;
//PWM PINS
const byte PWMA = 4;
const byte PWMB = 5;
//########################################################################

//Interrupt
void revDetectorISR() {
	isrRevCount++; //anzahl der interrupts +1
}

//Funktion zur berechnung der RPM
void Rotationverarbeitung() {
	if (isrRevCount != 0) { //wenn neue pulse erhalten
		noInterrupts();
		RPM = (30000000 / (micros() - timeold));
		isrRevCount = 0;
		timeold = micros();
		interrupts();
		Serial.println("3000,0," + RPM);
	}
}

//FULLY WORKING
void receiveEvent(int howMany) {
	int setPWMread = Wire.read();    // receive byte as an integer
	if (setPWMread != setPWM) { //ausführen wenn neuer wert angekommen
		analogWrite(PWMA, setPWM); //PWM ausführen
		setPWM = setPWMread;
	}
}

//FULLY WOKRING
void requestEvent() {
	long bigNum = RPM; //aufteilen der 4bytezahl (long) in bytes
	byte myArray[4];
	myArray[0] = (bigNum >> 24) & 0xFF;
	myArray[1] = (bigNum >> 16) & 0xFF;
	myArray[2] = (bigNum >> 8) & 0xFF;
	myArray[3] = bigNum & 0xFF;
	Wire.write(myArray, 4);
}


void setup() {
	//Serial
	Serial.begin(115200);
	//########################################################################
		//RPMMEssung
	pinMode(Lichtschrankepin, INPUT);
	attachInterrupt(0, revDetectorISR, FALLING);
	//########################################################################
		//PWM H-Brücke
	pinMode(PWMB, OUTPUT);
	analogWrite(PWMB, 0);
	pinMode(PWMA, OUTPUT);
	//########################################################################
		//I2C Connection
	Wire.begin(8);                // join i2c bus with address #8
	Wire.onReceive(receiveEvent); // register event
	Wire.onRequest(requestEvent); // register event
}

void loop() {
	Rotationverarbeitung();
}