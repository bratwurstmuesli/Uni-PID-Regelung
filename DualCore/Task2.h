//
void workLoad (void) ;                // prototype is required

void Task2( void * parameter )
{
  for (;;) {
    unsigned long start = millis();   // ref: https://github.com/espressif/arduino-esp32/issues/384

    workLoad();

    Serial << "Task 2 complete running on Core " << (xPortGetCoreID()) << " Time = " << (millis() - start) << " mS"  << endl ;

    delay(10) ;
  }
}
