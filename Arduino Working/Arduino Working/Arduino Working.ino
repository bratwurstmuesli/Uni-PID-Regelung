#include <Wire.h>

const byte Lichtschrankepin = 2;
const byte PWMA = 3;
const byte PWMB = 4;

//to calculate time between Pulses
unsigned long revMicros; //zeit dawzischen
unsigned long revCount;
unsigned long prevIsrMicros;
unsigned long latestIsrMicros;

// variables used by the ISR
volatile unsigned long isrRevMicros = 0; //zeit des neuesten Interrupts
volatile unsigned long isrRevCount = 0; //wie oft interrupt bisher
volatile bool isrRevNew = false;

//RPM
unsigned long RPM = 0;
bool rotationNew = false;

//Drive
int setPWM = 0;

void revDetectorISR() {
	isrRevMicros = micros(); //Zeit des Interrupts
	isrRevCount++; //anzahl der interrupts +1
	isrRevNew = true; //flag für neue Pulse
}

void Rotationverarbeitung() {

	if (isrRevNew == true) { //wenn neue pulse erhalten

		static unsigned long previousMillis = 0;
		const long interval = 100;
		unsigned long currentMillis = millis();
		if (currentMillis - previousMillis >= interval) {
			previousMillis = currentMillis;

			noInterrupts();
			//uebertrage Interruptzahl
			revCount = isrRevCount; //uebertrage isrinterrupts in normale variable
			isrRevCount = 0;
			//uebertrage neueste Interrupt Zeit
			latestIsrMicros = isrRevMicros; //setze den zeitpunkt der letzen berechneten Umdrehung der interruptroutine als neueste Zeit
			//berechne letzen Zeitpunkt
			revMicros = latestIsrMicros - prevIsrMicros; //calculate time difference
			//setze variable damit letzer berechnungspunkt klar
			prevIsrMicros = latestIsrMicros; //setze neuesten wert als letzen erhaltenen Wert
			//setze flag fuer nächste berechnung
			isrRevNew = false; //setze flag false
			rotationNew = true;
			interrupts();
		}
	}
}
void rotationstoRPM() {
	if (rotationNew == true) {
		//Serial.print(revMicros);
		//Serial.print(",");
		//Serial.println(revMicros / revCount);
		RPM = ((60 * 1000 * 1000) / (revMicros / revCount));
		//Serial.println(RPM);
		rotationNew = false;
	}
}

void receiveEvent(int howMany) {
	setPWM = Wire.read();    // receive byte as an integer
	Serial.println(setPWM);
}

void requestEvent() {

	int32_t bigNum = (unsigned)(long)RPM;
	Serial.println(bigNum);

	byte myArray[4];

	myArray[0] = (bigNum >> 24) & 0xFF;
	myArray[1] = (bigNum >> 16) & 0xFF;
	myArray[2] = (bigNum >> 8) & 0xFF;
	myArray[3] = bigNum & 0xFF;

	Wire.write(myArray, 4);
}

void setup() {
	Serial.begin(115200);
	pinMode(Lichtschrankepin, INPUT);
	attachInterrupt(0, revDetectorISR, FALLING);

	pinMode(PWMB, OUTPUT);
	digitalWrite(PWMB, LOW);
	analogWrite(PWMB, 0);

	pinMode(PWMA, OUTPUT);
	analogWrite(PWMA, 254);

	Wire.begin(8);                // join i2c bus with address #8
	Wire.onReceive(receiveEvent); // register event
	Wire.onRequest(requestEvent); // register event

	prevIsrMicros = micros();
}

void loop() {
	analogWrite(PWMA, setPWM);
	Rotationverarbeitung();
	rotationstoRPM();
}