const byte Lichtschrankepin = 2;
const byte PWMmosfet = 3;

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

void revDetectorISR() {
	isrRevMicros = micros(); //Zeit des Interrupts
	isrRevCount++; //anzahl der interrupts +1
	isrRevNew = true; //flag für neue Pulse
}

void Rotationverarbeitung() {
	if (isrRevNew == true) { //wenn neue pulse erhalten
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
		interrupts();
	}
}
void rotationstoRPM() {
	RPM = ((60 * 1000 * 1000) / (revMicros / revCount));
	Serial.println(RPM);
}

void setup() {
	Serial.begin(115200);
	pinMode(Lichtschrankepin, INPUT);
	attachInterrupt(0, revDetectorISR, FALLING);
}

void loop() {
	Rotationverarbeitung();
	rotationstoRPM();
}
