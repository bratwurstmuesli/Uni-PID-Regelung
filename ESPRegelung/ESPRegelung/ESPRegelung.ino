/*
 Name:		ESPRegelung.ino
 Created:	1/22/2019 2:48:12 PM
 Author:	Heiko
*/

//H-Bridge
//L9110
//5V!
//TWO Digital Outputs
const int speepin1 = 12;
const int speedpin2 = 14;
// setting PWM properties
const int freq = 40000;
const int ledChannel = 0;
const int ledChannel1 = 1;
const int resolution = 8;


//Stromstärke-Messung
//ACS712-5
//5V!
//Analog Input
//66 mV / A
//fließt kein Strom - Ausgangsspannung ist ~ VCC / 2
const int strompin = 25;

//Lichtschranke
//5V!
//Digtal Input
const int pulsepin = 33;
int rpm;

//https://techtutorialsx.com/2017/09/30/esp32-arduino-external-interrupts/



void speedcontrol(int pwm, char a) {
	if (a == 'f') {
		ledcWrite(ledChannel, pwm);
		//ledcWrite(ledChannel1, 0);
		digitalWrite(speedpin2, LOW);
	}
	else if (a == 'b') {
		//ledcWrite(ledChannel, 0);
		digitalWrite(speedpin2, LOW);
		ledcWrite(ledChannel1, pwm);
	}
	else {
		ledcWrite(ledChannel, 0);
		ledcWrite(ledChannel1, 0);
	}
}

//ON ICE
//void currentsense() {
//	https://www.allegromicro.com/~/media/files/datasheets/acs712-datasheet.ashx
//	int currentint = analogRead(strompin);
//		float current = (currentint)*(3.3/4096);
//		Serial.print("int: ");
//		Serial.println(currentint);
//		Serial.print("Volt: ");
//		Serial.println(current);
//}

const byte interruptPin = 33;
volatile int interruptCounter = 0;
int numberOfInterrupts = 0;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR handleInterrupt() {
	portENTER_CRITICAL_ISR(&mux);
	interruptCounter++;
	countpulse();
	portEXIT_CRITICAL_ISR(&mux);
}

void countpulse() {
	static unsigned long t2;
	//Serial.print("t2: ");
	//Serial.println(t2);
	if (t2 != 0) {
		unsigned long t1 = micros();
		//Serial.print("t1: ");
		//Serial.println(t1);
		unsigned long zeitdiff = t1 - t2;
		if (zeitdiff != 0) {
			//Serial.print("zeitdiff: ");
			//Serial.println(zeitdiff);
			rpm = 1*1000*1000 / zeitdiff; //sekunden*millisekunden*mikrosekunde
			//Serial.print("RPM: ");
			//Serial.println(rpm);
			t2 = micros();
		}
	}
	t2 = micros();
}

void setup() {
	Serial.begin(115200);
	Serial.println("TEST");

	//count pulse
	pinMode(interruptPin, INPUT);
	attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);

	//PWM einbauen
	//https://randomnerdtutorials.com/esp32-pwm-arduino-ide/
	//https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/ledc.html
	ledcSetup(ledChannel, freq, resolution);
	//ledcSetup(ledChannel1, freq, resolution);
	ledcAttachPin(speepin1, ledChannel);
	//ledcAttachPin(speedpin2, ledChannel1);

	pinMode(speedpin2, OUTPUT);

	//provisorisch
	speedcontrol(170, 'f');
}

void loop() {
	/*static unsigned long previousMillis = 0;
	unsigned long currentMillis = millis();
	const long interval = 100;
	if (currentMillis - previousMillis >= interval) {
		previousMillis = currentMillis;
		static int fadevalue = 100;
		static int fade = 1;
		if (fadevalue <= 255 && fade == 1) {
			fadevalue = fadevalue + 5;
			if (fadevalue >= 255) {
				fade = 0;
			}
		}
		else if (fadevalue >= 100 && fade == 0) {
			fadevalue = fadevalue - 5;
			if (fadevalue <= 100) {
				fade = 1;
			}
		}
		speedcontrol(fadevalue, 'f');
	}*/

	static unsigned long previousMillis1 = 0;
	unsigned long currentMillis1 = millis();
	const long interval1 = 500;
	if (currentMillis1 - previousMillis1 >= interval1) {
		previousMillis1 = currentMillis1;
		Serial.println(rpm);
	}

	if (interruptCounter > 0) {

		portENTER_CRITICAL(&mux);
		interruptCounter--;
		portEXIT_CRITICAL(&mux);

		numberOfInterrupts++;
		//Serial.print("An interrupt has occurred. Total: ");
		//Serial.println(numberOfInterrupts);
		//Serial.print("RPM: ");
		//Serial.println(countpulse());
	}
}
