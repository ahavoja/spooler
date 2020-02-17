void loop() {
	now=millis();
	
	// slew acceleration
	calcSpeed(0);
	static bool newDir0=0;
	if(spd[0]>0) newDir0=1; else
	if(spd[0]<0) newDir0=0;
	if(newDir0!=dir[0]){
		slew.shaft_dir(newDir0);
		dir[0]=newDir0;
	}
	setSpeed(0);

	// receive commands from PC through USB
	static unsigned long timeSerial=0;
	if(Serial.available()){
		timeSerial = now;
		interpretByte(Serial.read());
	}

	// disable larson scanner whenever motors turn
	serialActive = now-timeSerial<1000?1:0;

	// stop motors if no speed commands are received
	if(now - timeReceived > 1000){
		receptionActive=0;
		goal[0]=0; goal[1]=0; goal[2]=0;
	}else receptionActive=1;
	
	static unsigned long owl=0;
	if(now-owl>1000){
		owl=now;
		printDebug();
	}
}
