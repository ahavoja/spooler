// Interrupt Service Routine that automatically keeps stepping a motor
ISR(TIMER1_CAPT_vect){ // http://www.gammon.com.au/interrupts
	static bool man=0; // shall we step the motor next
	if(motOn && man){
		if(dir) ++pos;
		else --pos;
		PORTD ^= 1<<4; // https://www.arduino.cc/en/Reference/PortManipulation
	}
 	if(kid<boy) boy=kid; // if new speed is higher than before
	unsigned long small=160000; // set maximum ISR refresh period, so it runs often enough
	if(boy<=small){
		man=1;
		small=boy;
	}else man=0;
	boy-=small;
	if(boy==0) boy=kid;
	fox(small); // set timer to wait for next motor step
}

volatile byte rat=0;
ISR(TIMER1_OVF_vect){
	++rat;
}
