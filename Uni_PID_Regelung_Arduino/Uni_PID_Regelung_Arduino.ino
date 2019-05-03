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
//ausschalten nach bestimmter zeit wenn kein neuer i2c
//unsigned long zeitletztemeldung;

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
		timeold = micros(); //hier ist der anfang der naechsten umdrehung
		interrupts();
		Serial.println(RPM);
	}
	else if (vergangeneZeit > 1000000 && RPM != 0) { // um keine umdrehung zu erkennen
		RPM = 0;
	}
}

//FULLY WORKING
void receiveEvent(int howMany) {
	int setPWMread = Wire.read();    // receive byte as an integer
	//zeitletztemeldung = millis();
	Serial.println("neuer PWM: ");
	Serial.println(setPWMread);
	if (setPWMread != setPWM) { // nur ausführen wenn neuer wert angekommen
		if (setPWMread >= 0 && setPWMread < 256) { //wenn ein PWM Wert sonst 0 pwm ausführen
			setPWM = setPWMread;
			analogWrite(PWMA, setPWM); //PWM ausführen
		}
		else {
			analogWrite(PWMA, 0);
		}
	}
	if (setPWM >= 255) {
		digitalWrite(8, HIGH);
	}
	else {
		digitalWrite(8, LOW);
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
	digitalWrite(PWMB, 0);
	pinMode(PWMA, OUTPUT);
	//LED
	pinMode(8, OUTPUT);
	//########################################################################
		//I2C Connection
	Wire.begin(8);                // join i2c bus with address #8
	Wire.onReceive(receiveEvent); // register event
	Wire.onRequest(requestEvent); // register event
}

void loop() {
	Rotationverarbeitung();
	//if (millis() - zeitletztemeldung > 2000) { //ausschalten nach 2s ohne meldung vom ESP32
		//analogWrite(PWMA, 100);
	//}
	//Serial.print("2000,0,");
	/*Serial.print(RPM);*/
	//Serial.print(",");
	//Serial.println(setPWM);

}