void settings(){ // this function changes some settings of TMC2130
	// slewing driver settings
	slew.begin(); // Initiate pins and registeries
	slew.high_sense_R(1); // reference voltage for coil current sense resistors	1 = 0.18V			 0 = 0.32V
	slew.hold_current(1); // 0-31 standstill current per motor coil
	slew.run_current(4); // 0-31,		 0 = 30 mA per coil,		31 = 980 mA per coil
	slew.power_down_delay(30); // how long to wait after movement stops before reducing to hold current 0-255 = 0-4 seconds
	slew.hold_delay(3); // 0-15 how gradually it reduces to hold current. 0=fast change. 15=slow change.
	slew.stealthChop(1); // Enable extremely quiet stepping
	slew.standstill_mode(2); // 2 means coil shorted if I_HOLD=0, 1 means freewheeling, 0 means normal operation
	slew.stealth_autoscale(1);
	slew.microsteps(0); // we dont want any
	slew.interpolate(1); // automatic 256 x microstepping
	slew.double_edge_step(1); // step on both rising and falling edges
	slew.chopper_mode(0); // 0=spreadCycle 1=constant off time
}

void silentMode(){
	silent=1;
	Serial.println(F("Silent mode"));
	slew.stealth_max_speed(50); // switch stealthChop off if motor spins fast enough (meaning if time between two steps is less than this)
}

void fastMode(){
	silent=0;
	Serial.println(F("Fast mode"));
	slew.stealth_max_speed(10000);
}

void stopMotors(){
	goal=0;
	spd=0;
	setSpeed();
	Serial.println(F("Stop motors"));
}

// calculates new speed for motor and limits its acceleration
// call this function in every iteration of loop()
void calcSpeed(){
	static unsigned long viime=now;
	const byte tuokio=now-viime;
	const byte increment=tuokio*acceleration;
	if(increment>0){
		viime=now;
		if(spd<goal){
			if(increment<goal-spd) spd+=increment;
			else spd=goal;
		}
		else if(spd>goal){
			if(increment<spd-goal) spd-=increment;
			else spd=goal;
		}
	}
}

// changes motor speed
void setSpeed(){
	if (spd==0){
		cli();
		motOn=0; // dont step the motor
		kid=0xFFFF00; // longest possible step period
		sei();
	}else{
		unsigned long newKid=16000000UL/abs(spd); // how many CPU cycles to wait between steps
		if(newKid>0xFFFF00) newKid=0xFFFF00; // why would we even try to step slower than this
		cli();
		motOn=1;
		kid=newKid;
		sei();
	}
}

// function to set timer1 period. Stolen from https://www.pjrc.com/teensy/td_libs_TimerOne.html
inline void fox(unsigned long cycles){
	if(cycles<1100) cycles=1100; // minimum cycles, cuz ISR takes some time too
	byte clockSelectBits;
	word period;
	if (cycles < 0x10000) {
		clockSelectBits = _BV(CS10);
		period = cycles;
	} else
	if (cycles < 0x10000 * 8) {
		clockSelectBits = _BV(CS11);
		period = cycles / 8;
	} else
	if (cycles < 0x10000 * 64) {
		clockSelectBits = _BV(CS11) | _BV(CS10);
		period = cycles / 64;
	} else
	if (cycles < 0x10000 * 256) {
		clockSelectBits = _BV(CS12);
		period = cycles / 256;
	} else {
		clockSelectBits = _BV(CS12);
		period = 0xFFFF;
	}
	ICR1 = period;
	TCCR1B = _BV(WGM13) | _BV(WGM12) | clockSelectBits; // mode 12. p136
	//TCNT1 = 0; // reset counter. p115
}

// Unlike its Arduino counterpart, this does not wait that ADC is ready. It makes using analogRead() inside interrupt so much faster.
int analogRead(byte pin){
	if (pin >= 14) pin -= 14; // allow for channel or pin numbers
	if (pin>7) return -1;
	static int result[8]={0};
	static byte lastPin=0;
	if(bit_is_clear(ADCSRA, ADSC)){ // ADSC is cleared when the conversion finishes.
		result[lastPin] = ADCL | (ADCH << 8);
		ADMUX = (DEFAULT << 6) | (pin & 0x07);
		lastPin=pin;
		bitSet(ADCSRA, ADSC); // start the conversion
	}
	return result[pin];
}

// Read accelerations from memory
void readAccels(){
	Serial.print(F("Accel: "));
	word accel;
	EEPROM.get(4,accel);
	acceleration=accel/1000.;
	Serial.print(accel); // in units of steps/(s^2)
	Serial.println();
}

/*Start byte bit structure:
*1  always 1 in start byte
*0  for speed command or 1 for acceleration setting
*1  usually 1, but 0 for emergency stop
*0  1 to start homing
*0  for fast mode, 1 for silent mode
*0  1 for illumination
*0  15th bit of speed command, when 0th is LSB
*0  14th bit of speed command
*/

// Commands from USB and Ethernet are passed to this function, which then adjusts motor speeds and settings accordingly
void interpretByte(const byte wax){
	timeReceived = now;
	static byte job=255;
	static int newSpeed=0;
	if(wax & 0b10000000){ // start byte
		if(wax & 0b100000){ // no emergency stop
			if((wax & 0b1000000) == 0){ // speed command
				newSpeed=(wax&0b11)<<14;
				job=0;
			}
		}else stopMotors(); // emergency stop
		if(wax & 0b1000000) job=7; // acceleration setting
		if(wax & 0b1000){
			if(silent==0) silentMode();
		}else{
			if(silent==1) fastMode();
		}
	}
	else if(job<2){ // speed command
		if(job==0){
			newSpeed |= wax<<7;
		}else if(job==1){
			newSpeed |= wax;
			goal=newSpeed;
		}
		++job;
	}
	else if(job>6 && job<9){ // acceleration setting
		if(job==7) newSpeed=wax<<7;
		else if(job==8){
			newSpeed |= wax;
			EEPROM.put(4,newSpeed);
			readAccels();
		}
		++job;
	}
}

void setup() {
	DDRD |= 0b00010000; // step pins outputs
	Serial.begin(250000); // Set baud rate in serial monitor
	// let's enable timer1 to time the step pulses. See p113 here https://www.sparkfun.com/datasheets/Components/SMD/ATMega328.pdf
	TCCR1A=B00000000; //p134
	TIMSK1=B00100001; //p139
	fox(1000);
	fastMode();
	readAccels();
}

/*EEPROM map:
*bytes 0,1,2,3 reserved for IP-address
*4,5 slew acceleration
*6,7 trolley acceleration
*8,9 hook acceleration
*/
