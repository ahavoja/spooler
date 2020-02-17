// Interrupt Service Routine that automatically keeps stepping motors
ISR(TIMER1_CAPT_vect){ // http://www.gammon.com.au/interrupts
	static bool man[3]={0}; // which motors to step next
	
	// slew
	if(motOn[0] && man[0]){
		if(dir[0]) ++pos[0];
		else --pos[0];
		PORTD ^= 1<<4; // https://www.arduino.cc/en/Reference/PortManipulation
	}

	// if new speed is higher than before
 	if(kid[0]<boy[0]) boy[0]=kid[0];
 	if(kid[1]<boy[1]) boy[1]=kid[1];
 	if(kid[2]<boy[2]) boy[2]=kid[2];

	// find who is the smallest boy
	unsigned long small=160000; // set maximum ISR refresh period, so it runs often enough
	if(boy[0]<=boy[1] && boy[0]<=boy[2] && boy[0]<=small){
		man[0]=1;
		small=boy[0];
	}else man[0]=0;
	if(boy[1]<=boy[0] && boy[1]<=boy[2] && boy[1]<=small){
		man[1]=1;
		small=boy[1];
	}else man[1]=0;
	if(boy[2]<=boy[0] && boy[2]<=boy[1] && boy[2]<=small){
		man[2]=1;
		small=boy[2];
	}else man[2]=0;

	// update boys
	boy[0]-=small;
	boy[1]-=small;
	boy[2]-=small;
	if(boy[0]==0) boy[0]=kid[0];
	if(boy[1]==0) boy[1]=kid[1];
	if(boy[2]==0) boy[2]=kid[2];
	
	fox(small); // set timer to wait for next motor step
}

volatile byte rat=0;
ISR(TIMER1_OVF_vect){
	++rat;
}
