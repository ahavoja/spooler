// homing function
void home(){
	if(homing==1){ // start homing
		fastMode();
		acceleration[0]=3.0;
		acceleration[1]=2.0;
		acceleration[2]=0.5;
		Serial.println(F("Lowering hook"));
		posTop=0x7FFFFFFF;
		goal[0]=0;
		goal[1]=0;
		goal[2]=-200; // lower hook a bit so that it surely has room to accelerate upwards for stallguard to work
		pos[2]=0;
		hookHitGround=0;
		homing=2;
	}else if(homing==2){
		if(pos[2]<=-20 || hookHitGround){ // start raising hook if it has gone low enough or has hit ground
			Serial.println(F("Raising hook"));
			goal[2]=200;
			homing=3;
		}
	}else if(homing==4){
		delay(100); // wait a bit for vibrations from hook hitting trolley to attenuate
		hook.shaft_dir(!dir[2]); // lower hook a bit to relieve tension in rope
		for(byte i=0; i<9; i++){
			delay(10);
			PORTD ^= 1<<6;
		}
		hook.shaft_dir(dir[2]);
		pos[2]=0;
		posTop=0;
		Serial.println(F("Hook homed. Homing trolley and slew"));
		posMin=0x80000000;
		homeSlew=1;
		goal[0]=2400;
		homeTrolley=1;
		goal[1]=-600;
		homing=5;
	}else if(homing==5){
		if(homeTrolley==2){
			Serial.println(F("Edge detected"));
			posMin=0;
			pos[1]=-10; // stop before edge
			posMax=0x7FFFFFFF;
			delay(100); // vibration dampening time
			goal[1]=600; // change direction
			homeTrolley=3;
		}else if(homeTrolley==4){
			Serial.println(F("Trolley homed"));
			posMax=pos[1];
			homeTrolley=0;
		}
		if(homeSlew==3){
			goal[0]=0;
			Serial.println(F("Slew homed"));
			homeSlew=0;
		}
		if(homeSlew==0 && homeTrolley==0 && spd[0]==0 && spd[1]==0 && spd[2]==0){
			Serial.println(F("Homing finished"));
			readAccels(); // set accelerations back to original
			homing=0;
		}
	}
}
