#include <Arduino.h>


// ARDUINO PIN CONFIGURATION --------------------------------
// Stepper motor pins
const int  STEP_PIN1 = 8;
const int  STEP_PIN2 = 9;
const int  STEP_PIN3 = 10;
const int  STEP_PIN4 = 11;

// Potentiometer wiper set to ADC A0


// CONSTANTS -------------------------------------------------
const int Mot_OFF = 99; // Turn off stepper motor
const int Mot_REV = 98; // Reverse stepper motor revolution

// GLOBAL VARIABLES -------------------------------------------
int ADC_val = 0;  // variable to store the value read

int CMD_Val; // Command Numerical value

int SV_Metering_Valve; // Set Value (ADC Value) to reach for Metering valve (0-1023)
int CV_Metering_Valve; // Current Value (ADC Value) of Metering valve (0-1023)

byte digital_out_status; // Digital output (relay board) register

// GLOBAL STRINGS -------------------------------------------
String inString = ""; // for incoming serial string data

// GLOBAL FLAGS
bool motor_flag = 0; // flag for activating motor control


// EEPROM MEMORY ADDRESSES
// 0x00 MSB ADC_val
// 0x01 LSB ADC_val
// 0x02	Digital Output (relay board) status --> copy of digital_out_status register

void Output_state( int i4,int i3,int i2,int i1)
{
  if (i1==1) digitalWrite(STEP_PIN1,HIGH); else digitalWrite(STEP_PIN1,LOW);
  if (i2==1) digitalWrite(STEP_PIN2,HIGH); else digitalWrite(STEP_PIN2,LOW);
  if (i3==1) digitalWrite(STEP_PIN3,HIGH); else digitalWrite(STEP_PIN3,LOW);
  if (i4==1) digitalWrite(STEP_PIN4,HIGH); else digitalWrite(STEP_PIN4,LOW);
}

void Step(int state)
{
int i1,i2,i3,i4;

  switch (state)
    {
       case 0: Output_state(0,0,0,1); break;
       case 1: Output_state(0,0,1,1); break;
       case 2: Output_state(0,0,1,0); break;
       case 3: Output_state(0,1,1,0); break;
       case 4: Output_state(0,1,0,0); break;
       case 5: Output_state(1,1,0,0); break;
       case 6: Output_state(1,0,0,0); break;
       case 7: Output_state(1,0,0,1); break;
       case Mot_OFF: //OFF
          Output_state(0,0,0,0); break;
    }
   delay(1);
}

void setup() {
    // Digital Pins setup
    pinMode(STEP_PIN1, OUTPUT);
    pinMode(STEP_PIN2, OUTPUT);
    pinMode(STEP_PIN3, OUTPUT);
    pinMode(STEP_PIN4, OUTPUT);

    // Serial port setup
    Serial.begin(9600);
}

void loop() {
	// Local Variables
	int state, prescaler;

	// Check for new data from serial port --------------------------------------------------------
	while (Serial.available()) {
	delay(10);
	if (Serial.available() >0) {
		char c = Serial.read();
		inString += c; // Read data
		}
	}
	// Check command from serial port --------------------------------------------------------
	if (inString.length()> 0) {
		char CMD = inString.charAt(0);
		switch (CMD) {
			case 'A': // Read ADC
			      ADC_val = analogRead(A0);
			      Serial.print("ADC Value: ");
			      Serial.println(ADC_val);
				break;
			case 'M': // Metering Valve Motor Control
				inString.setCharAt(0, '0');
				SV_Metering_Valve = inString.toInt();
				// Check ADC boundaries
				if (SV_Metering_Valve > 0 && SV_Metering_Valve < 1023){
					motor_flag = 1;
				}
				else{
					Serial.println("Motor SV out of range");
					SV_Metering_Valve = analogRead(A0); // Get current ADC value
				}
				break;
			case 'S': // Solenoid Valve Control
				inString.setCharAt(0, '0');
				digital_out_status = inString.toInt();
						if (digital_out_status <= 15 && digital_out_status >= 0){
							Serial.println(digital_out_status);
						}
				break;
			default:
				Serial.println("Command not recognized");
				break;
		}

		// Clear string for next command
		inString = "";
	}

	// Turn Metering valve motor ------------------------------------------------------------
	if (motor_flag){
		motor_flag = 0; // Reset flag
		CV_Metering_Valve = analogRead(A0); // Get current ADC value
		if (SV_Metering_Valve > CV_Metering_Valve){
			// CW motor turn
			while (CV_Metering_Valve < SV_Metering_Valve){
				for(int i = 7; i>-1; i--)
				{
					Step(i);
					if (SV_Metering_Valve - CV_Metering_Valve <5){
						delay(10);
					}
				}
				CV_Metering_Valve = analogRead(A0); // Get current ADC value
			}
		}
		if (SV_Metering_Valve < CV_Metering_Valve){
			// CCW motor turn
			while (CV_Metering_Valve > SV_Metering_Valve){
				for(int i = 0; i<8; i++)
				{
					Step(i);
					if (CV_Metering_Valve - SV_Metering_Valve <5){
						delay(10);
					}
				}
				CV_Metering_Valve = analogRead(A0); // Get current ADC value
			}
		}
		Serial.println("*");
	}
}
