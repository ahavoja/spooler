void loop() {
	now=millis();
	
	// motor acceleration
	calcSpeed();
	static bool newDir=0;
	if(spd>0) newDir=1; else
	if(spd<0) newDir=0;
	if(newDir!=dir){
		slew.shaft_dir(newDir);
		dir=newDir;
	}
	setSpeed();

	// receive commands from PC through USB
	if(Serial.available()) interpretByte(Serial.read());

	// stop motor if no speed commands are received
	if(now - timeReceived > 1000) goal=0;
	
	static unsigned long owl=0;
	if(now-owl>1000){
		owl=now;
		printDebug();
	}
}
