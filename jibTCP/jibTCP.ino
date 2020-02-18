// This code reads commands from serial port (USB wire) and controls a stepper motor via TMC2130 driver.
// This code runs on atmega 328p microcontroller (Arduino Uno or Nano)
/*todo:
* millis or something causes jitter in motor movement every second
* receive speed commands larger than 8000
*/

// TMC2130 pin connections
	/* You need to connect the SPI pins as follows for programming the TMC2130. If you have several TMC2130, they all must use these same pins.
		SDI --> D11
		SDO --> D12
		SCK --> D13
	En --> GND // enable (CFG6). I want driver always enabled, so connect EN --> GND
	Dir --> GND // direction can also be controlled through SPI, so to save pins, connect DIR --> GND
	Step (slew) --> D4 
	Step (trolley) --> D5
	Step (hook) --> D6
	And on top of that you need to also connect motor coils
		M1A and M1B to one coil and
		M2A and M2B to another coil
	Finally connect the power wires
		GND --> GND
		VIO --> 5V
		VM --> motor power supply (5 - 45 V) and > 100 µF capacitor */

// At 24 V input voltage, slewing motor can spin max 7700 steps/second before stalling.

// a motor can never spin too fast, right?
#pragma GCC optimize ("-O2") // https://www.instructables.com/id/Arduino-IDE-16x-compiler-optimisations-faster-code/

// This code uses libraries. These can be easily installed through Arduino IDE library manager by pressing CTRL + SHIFT + I
#include <TMC2130Stepper.h> // https://github.com/teemuatlut/TMC2130Stepper

// choose chip select pins for each stepper driver
TMC2130Stepper slew = TMC2130Stepper(A0);

#include <EEPROM.h> // for storing unique IP address for each arduino

// global variables
volatile unsigned long
	kid=0xFFFF00, // CPU cycles to wait between steps for motor
	boy=0xFFFF00; // CPU cycles left until the motor needs to be stepped again
volatile bool motOn=0; // is the motor spinning
volatile bool dir=0; // motor direction
volatile long pos=0; // motor step position
int spd=0, goal=0;
bool silent;
unsigned long now; // current time in loop()
unsigned long timeReceived=0; // when was last byte received via USB
float acceleration; // stores slew, trolley, hook acceleration limits
