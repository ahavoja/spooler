void printDebug()
{
	bool say=0;
	//Serial.print(analogRead(A7)); // print hall sensor readings
	
	long positron[3];
	static long positronOld[3]={0,0,0};
	for(byte i=0; i<3; i++){ // copy motor positions to buffer
		cli();
		positron[i]=pos[i];
		sei();
		if (positron[i] != positronOld[i]){
			say=1;
			positronOld[i]=positron[i];
		}
	}
	
	const int stallGuardSlew = slew.DRV_STATUS() & 0x3FF;
		
	const float Vin=analogRead(A7)*0.03812; // input voltage
	static float VinOld=0;
	if(Vin < VinOld-0.3 || Vin > VinOld+0.3){ // hysteresis to not print voltage ripple. 0.3 works with 1 µF and 68 kΩ
		say=1;
		VinOld=Vin;
	}

	static char spdOld[3]={0,0,0};
	for(byte i=0; i<3; i++){
		if(spd[i] != spdOld[i]){
			say=1;
			spdOld[i]=spd[i];
		}
	}

	if(rat>0) say=1;
	
	if(say){
		for(byte i=0; i<3; i++){ // print motor positions
			Serial.print(positron[i]);
			Serial.print(",");
		}
		Serial.print("  ");
		for(byte i=0; i<3; i++){ // print motor speeds
			Serial.print(spd[i],DEC);
			Serial.print(",");
		}
		Serial.print("  ");
		Serial.print(stallGuardSlew, DEC);
		Serial.print(",  ");
		Serial.print(Vin,1);
		Serial.print(", ");
		Serial.print(analogRead(A6));
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
