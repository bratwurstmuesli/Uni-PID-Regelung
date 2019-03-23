/*
   This sketch runs the same load on both cores of the ESP32
   ArduinoIDE 1.8.5
   Linux Mint 18.3
    Sketch uses 162569 bytes (12%) of program storage space. Maximum is 1310720 bytes.
    Global variables use 11068 bytes (3%) of dynamic memory, leaving 283844 bytes for local variables. Maximum is 294912 bytes.
*/

#include <Streaming.h>      // Ref: http://arduiniana.org/libraries/streaming/
#include "Workload.h"
#include "Task1.h"
#include "Task2.h"

TaskHandle_t TaskA, TaskB;

void setup() {
  Serial.begin(115200);
  delay(500);  // small delay
  
  // Ref: http://esp32.info/docs/esp_idf/html/db/da4/task_8h.html#a25b035ac6b7809ff16c828be270e1431
  
  xTaskCreatePinnedToCore(
    Task1,                  /* pvTaskCode */
    "Workload1",            /* pcName */
    1000,                   /* usStackDepth */
    NULL,                   /* pvParameters */
    1,                      /* uxPriority */
    &TaskA,                 /* pxCreatedTask */
    0);                     /* xCoreID */

  xTaskCreatePinnedToCore(
    Task2,
    "Workload2",
    1000,
    NULL,
    1,
    &TaskB,
    1);
}

void loop() {
  // This task will run in the ESP32 Arduino default context
  unsigned long start = millis();
  Serial << "Task 0 complete running on Core " << (xPortGetCoreID()) << " Time = " << (millis() - start) << " mS"  << endl ;;
  delay(10) ;
}
