
#include "arduino.h"

#include "counter.h"


ChainedTimer<uint8_t> terminator;  // 56 bytes

/**
 * This is the source of the timer events.                                                                     
 */
MillisCounter ms;   // 104 bytes

Multiplier<uint8_t, uint16_t, uint8_t> tenthSeconds(100, ms);
Multiplier<uint8_t, uint8_t, uint8_t> seconds(10, tenthSeconds);
Multiplier<uint8_t, uint8_t, uint8_t> minutes(60, seconds);



void setup()
{
	long l = millis();
				
	//delay(l);
}

void loop()
{
	#if 1
	//delay(10);
	ms.update();
	
	if (tenthSeconds >= 1) {
		tenthSeconds.reset();
	}
	#endif
}


int main(void) __attribute__((noreturn));

int main(void) 
{
	init();
    setup();
	
	while(1) {
		loop();
	}	
}