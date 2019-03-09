/*
 Name:		ESPRegelung.ino
 Created:	1/22/2019 2:48:12 PM
 Author:	Heiko
*/

TaskHandle_t Task1;
TaskHandle_t Task2;



//H-Bridge
//L9110
//5V!
//TWO Digital Outputs
const int speepin1 = 12;
const int speedpin2 = 14;
// setting PWM properties
const int freq = 5000;
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

//https://techtutorialsx.com/2017/09/30/esp32-arduino-external-interrupts/



void speedcontrol(int pwm, char a) {
	if (a == 'f') {
		ledcWrite(ledChannel, pwm);
		ledcWrite(ledChannel1, 0);
	}
	else if (a == 'b') {
		ledcWrite(ledChannel, 0);
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

int countpulse() {
	static unsigned long t2;
	//Serial.print("t2: ");
	//Serial.println(t2);
	if (t2 != 0) {
		unsigned long t1 = micros();
		//Serial.print("t1: ");
		//Serial.println(t1);
		unsigned long zeitdiff = t1 - t2;
		if (zeitdiff != 0) {
			Serial.print("zeitdiff: ");
			Serial.println(zeitdiff);
			long rpm = 6000000 / zeitdiff;
			Serial.print("RPM: ");
			Serial.println(rpm);
			t2 = micros();
			return rpm;
		}


	}
	t2 = micros();

}

void setup() {
	xTaskCreatePinnedToCore(
		Task1code,   /* Task function. */
		"Task1",     /* name of task. */
		10000,       /* Stack size of task */
		NULL,        /* parameter of the task */
		1,           /* priority of the task */
		&Task1,      /* Task handle to keep track of created task */
		0);          /* pin task to core 0 */
	delay(500);

	//create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
	xTaskCreatePinnedToCore(
		Task2code,   /* Task function. */
		"Task2",     /* name of task. */
		10000,       /* Stack size of task */
		NULL,        /* parameter of the task */
		1,           /* priority of the task */
		&Task2,      /* Task handle to keep track of created task */
		1);          /* pin task to core 1 */
	delay(500);


	Serial.begin(115200);
	Serial.println("TEST");

	//count pulse
	pinMode(interruptPin, INPUT);
	attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);

	//PWM einbauen
	//https://randomnerdtutorials.com/esp32-pwm-arduino-ide/
	//https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/ledc.html
	ledcSetup(ledChannel, freq, resolution);
	ledcSetup(ledChannel1, freq, resolution);
	ledcAttachPin(speepin1, ledChannel);
	ledcAttachPin(speedpin2, ledChannel1);

	//provisorisch
	speedcontrol(180, 'f');
}

void loop() {



}

void Task1code(void * pvParameters) {
	Serial.print("Task1 running on core ");
	Serial.println(xPortGetCoreID());

	for (;;) {
		//if (interruptCounter > 0) {

		//	portENTER_CRITICAL(&mux);
		//	interruptCounter--;
		//	portEXIT_CRITICAL(&mux);

		//	numberOfInterrupts++;
		//	//Serial.print("An interrupt has occurred. Total: ");
		//	//Serial.println(numberOfInterrupts);
		//	//Serial.print("RPM: ");
		//	//Serial.println(countpulse());
		//}
	}
}

//Task2code: blinks an LED every 700 ms
void Task2code(void * pvParameters) {
	Serial.print("Task2 running on core ");
	Serial.println(xPortGetCoreID());

	for (;;) {
		for (int fadeValue = 255; fadeValue >= 0; fadeValue -= 5) {
			// sets the value (range from 0 to 255):
			speedcontrol(fadeValue, 'f');
			// wait for 30 milliseconds to see the dimming effect
			delay(30);
		}
		for (int fadeValue = 0; fadeValue <= 255; fadeValue += 5) {
			// sets the value (range from 0 to 255):
			speedcontrol(fadeValue, 'f');
			// wait for 30 milliseconds to see the dimming effect
			delay(30);
		}
	}
}
