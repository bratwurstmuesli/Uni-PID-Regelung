/*
 Name:		ESPRegelung.ino
 Created:	1/22/2019 2:48:12 PM
 Author:	Heiko
*/
#include <PID_v1.h>

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

//speedpot
const int speedpotpin = 34;

//Stromst‰rke-Messung
//ACS712-5
//5V!
//Analog Input
//66 mV / A
//flieﬂt kein Strom - Ausgangsspannung ist ~ VCC / 2
const int strompin = 25;

//Lichtschranke
//5V!
//Digtal Input
const int dataIN = 33; //IR sensor INPUT

volatile unsigned long prevmillis; // To store time
volatile unsigned long duration; // To store time difference
volatile unsigned long refresh; // To store time for refresh of reading

volatile int rpm; // RPM value

boolean currentstate; // Current state of IR input scan
boolean prevstate; // State of IR sensor in previous scan
//https://www.instructables.com/id/Simple-Motor-Speed-Tester-Tachometer/

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
double Kp = 2, Ki = 5, Kd = 1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

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

//ON ICE
void currentsense() {
https://www.allegromicro.com/~/media/files/datasheets/acs712-datasheet.ashx
	int currentint = analogRead(strompin);
	//runningAverage(currentint);
	float current = (currentint)*(3.3 / 4096);
	Serial.print("int: ");
	Serial.println(currentint);
	Serial.print("Volt: ");
	Serial.println(current);
}

//const byte interruptPin = 33;
//volatile int interruptCounter = 0;
//int numberOfInterrupts = 0;

//portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
//
//void IRAM_ATTR handleInterrupt() {
//	portENTER_CRITICAL_ISR(&mux);
//	interruptCounter++;
//	portEXIT_CRITICAL_ISR(&mux);
//}

//void countpulse() {
//	static unsigned long t2;
//	//Serial.print("t2: ");
//	//Serial.println(t2);
//	if (t2 != 0) {
//		unsigned long t1 = millis();
//		//Serial.print("t1: ");
//		//Serial.println(t1);
//		if (t2 < t1 && numberOfInterrupts != 0) {
//			unsigned long zeitdiff = t1 - t2;
//			//Serial.print("numberOfInterrupts: ");
//			//Serial.println(numberOfInterrupts);
//			int mittelwertrotatation = zeitdiff / numberOfInterrupts;
//			//int rpm = (60 * 1000) / mittelwertrotatation;
//			//Serial.print("rpm: ");
//			//Serial.println(rpm);
//			numberOfInterrupts = 0;
//		}
//	}
//	t2 = millis();
//}

void rpmmeasure() { // RPM Measurement
	currentstate = digitalRead(dataIN); // Read IR sensor state
	if (prevstate != currentstate) // If there is change in input
	{
		if (currentstate == HIGH) // If input only changes from LOW to HIGH
		{
			duration = (micros() - prevmillis); // Time difference between revolution in microsecond
			rpm = (60000000 / duration); // rpm = (1/ time millis)*1000*1000*60;
			prevmillis = micros(); // store time for nect revolution calculation
		}
	}
	prevstate = currentstate; // store this scan (prev scan) data for next scan
}


void setspeed() {
	int analogvalue = analogRead(speedpotpin);
	int mapvalue = map(analogvalue, 0, 4096, 0, 255);
	speedcontrol(mapvalue, 'f');
	//Serial.println(analogvalue);
}

//TaskHandle_t Task1;
//TaskHandle_t Task2;

void setup() {
	Serial.begin(115200);
	Serial.println("TEST");

	pinMode(dataIN, INPUT);
	prevmillis = 0;
	prevstate = LOW;

	////create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
	//xTaskCreatePinnedToCore(
	//	Task1code,   /* Task function. */
	//	"Task1",     /* name of task. */
	//	10000,       /* Stack size of task */
	//	NULL,        /* parameter of the task */
	//	1,           /* priority of the task */
	//	&Task1,      /* Task handle to keep track of created task */
	//	0);          /* pin task to core 0 */
	//delay(500);
	////create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
	//xTaskCreatePinnedToCore(
	//	Task2code,   /* Task function. */
	//	"Task2",     /* name of task. */
	//	10000,       /* Stack size of task */
	//	NULL,        /* parameter of the task */
	//	1,           /* priority of the task */
	//	&Task2,      /* Task handle to keep track of created task */
	//	1);          /* pin task to core 1 */
	//delay(500);

	//PID
	Setpoint = 1000;
	//turn the PID on
	myPID.SetMode(AUTOMATIC);

	//Interrupt
		/*pinMode(interruptPinhigh, INPUT);*/

		//attachInterrupt(digitalPinToInterrupt(dataIN), handleInterrupt, FALLING); //falling normally
		//attachInterrupt(digitalPinToInterrupt(interruptPinhigh), handleInterrupthigh, HIGH);


		//PWM einbauen
		//https://randomnerdtutorials.com/esp32-pwm-arduino-ide/
		//https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/ledc.html
	ledcSetup(ledChannel, freq, resolution);
	ledcAttachPin(speepin1, ledChannel);
	pinMode(speedpin2, OUTPUT);

	//provisorisch
	//speedcontrol(255, 'f');
	//delay(5000);
}

//void Task1code(void * pvParameters) {
//	Serial.print("Task1 running on core ");
//	Serial.println(xPortGetCoreID());
//	
//	for (;;) {
//
//	}
//}
//
//void Task2code(void * pvParameters) {
//	Serial.print("Task2 running on core ");
//	Serial.println(xPortGetCoreID());
//	
//	for (;;) {
//	
//	}
//}

void loop() {
	//rpmmeasure();
	currentstate = digitalRead(dataIN); // Read IR sensor state
	if (prevstate != currentstate) // If there is change in input
	{
		if (currentstate == HIGH) // If input only changes from LOW to HIGH
		{
			duration = (micros() - prevmillis); // Time difference between revolution in microsecond
			rpm = (60000000 / duration); // rpm = (1/ time millis)*1000*1000*60;
			prevmillis = micros(); // store time for nect revolution calculation
		}
	}
	prevstate = currentstate; // store this scan (prev scan) data for next scan

	Input = rpm;
	myPID.Compute();
	speedcontrol(Output, 'f');

	//Alle 100ms
	static unsigned long previousMillis = 0;
	unsigned long currentMillis = millis();
	const long interval = 1000;
	if (currentMillis - previousMillis >= interval) {
		previousMillis = currentMillis;
		int analogvalue = analogRead(speedpotpin);
		Setpoint = map(analogvalue, 0, 4096, 0, 2000);

		//setspeed();
		/*static int fadevalue = 100;
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
		speedcontrol(fadevalue, 'f');*/
	}



	// LCD Display
	//if ((millis() - refresh) >= 100)
	//{
	//	Serial.println(rpm);
	//}
	//currentsense();

	//alle 500ms
	static unsigned long previousMillis1 = 0;
	unsigned long currentMillis1 = millis();
	const long interval1 = 500;
	if (currentMillis1 - previousMillis1 >= interval1) {
		previousMillis1 = currentMillis1;
		Serial.print("Setpoint: ");
		Serial.println(Setpoint);
		Serial.print("rpm: ");
		Serial.println(rpm);
		Serial.println();

		//Serial.println(rpm);
	}
}
