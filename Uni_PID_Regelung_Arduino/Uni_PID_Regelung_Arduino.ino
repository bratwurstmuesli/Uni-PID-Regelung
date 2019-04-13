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
	unsigned long vergangeneZeit = micros() - timeold;
	if (isrRevCount != 0) { //wenn neue pulse erhalten
		noInterrupts();
		RPM = 30000000 / vergangeneZeit;
		isrRevCount = 0;
		timeold = micros();
		interrupts();
	}
	else if (vergangeneZeit > 1000000 && RPM != 0) { // um keine umdrehung zu erkennen
		RPM = 0;
	}
}

//FULLY WORKING
void receiveEvent(int howMany) {
	int setPWMread = Wire.read();    // receive byte as an integer
	//Serial.println(setPWMread);
	if (setPWMread != setPWM) { // nur ausführen wenn neuer wert angekommen
		if (setPWMread >= 0 && setPWMread < 256) { //wenn ein PWM Wert sonst 0 pwm ausführen
			setPWM = setPWMread;
			analogWrite(PWMA, setPWM); //PWM ausführen
		}
		else {
			analogWrite(PWMA, 0);
		}
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
	Serial.println("test Arduino");
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
	//Serial.print("3000,0,");
	//Serial.print(RPM);
	//Serial.print(",");
	//Serial.println(setPWM);
}