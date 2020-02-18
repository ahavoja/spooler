void printDebug()
{
	bool say=0;

	long positron;
	static long positronOld=0;
	// copy motor position to buffer
	cli();
	positron=pos;
	sei();
	if (positron != positronOld){
		say=1;
		positronOld=positron;
	}

	const int stallGuardSlew = slew.DRV_STATUS() & 0x3FF;

	const float Vin=analogRead(A7)*0.03812; // input voltage
	static float VinOld=0;
	if(Vin < VinOld-0.3 || Vin > VinOld+0.3){ // hysteresis to not print voltage ripple. 0.3 works with 1 µF and 68 kΩ
		say=1;
		VinOld=Vin;
	}

	static char spdOld=0;
	if(spd != spdOld){
		say=1;
		spdOld=spd;
	}

	if(rat>0) say=1;

	if(say){
		Serial.print(positron);
		Serial.print(",  ");
		Serial.print(spd,DEC);
		Serial.print(",  ");
		Serial.print(stallGuardSlew, DEC);
		Serial.print(",  ");
		Serial.print(Vin,1);
		if(rat>0){
			Serial.print(F(", timer1 overflow "));
			Serial.print(rat);
			Serial.print(F(" times"));
			rat=0;
		}
		Serial.println();
	}

	static bool enabled=0;
	if(enabled){
		if(Vin<5){
			Serial.println(F("Drivers off"));
			enabled=0;
		}
	}
	else if(Vin>6){ // auto re-enable drivers after power off
		Serial.println(F("Drivers on"));
		settings(); // this function blocks for half a second
		enabled=1;
	}
}
