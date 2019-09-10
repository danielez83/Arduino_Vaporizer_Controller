// ChangeLog
// 24-08-2019 Saved as a GiT repository

#include <Arduino.h>
#include "metering_valve.h"
#include "relay_board.h"
#include "SoftwareSerial.h" // by Paul Stoffregen (https://github.com/PaulStoffregen/SoftwareSerial)


// ARDUINO PIN CONFIGURATION --------------------------------
// Potentiometer wiper --> set to ADC A0
// IN1 ULN2003AN board  --> set to DIGITAL D8
// IN2 ULN2003AN board  --> set to DIGITAL D9
// IN3 ULN2003AN board  --> set to DIGITAL D10
// IN4 ULN2003AN board  --> set to DIGITAL D11

// RS485_to_RS232_MCU tx  --> set to DIGITAL D12 (SoftwareSerial TX)
// RS485_to_RS232_MCU rx  --> set to DIGITAL D13 (SoftwareSerial TX)


// CONSTANTS -------------------------------------------------


// GLOBAL VARIABLES -------------------------------------------
int ADC_val0 = 0;  // variable to store the potentiometer value
int ADC_val1 = 0;  // variable to store the LM735 value

int CMD_Val; // Command Numerical value

int SV_Metering_Valve; // Set Value (ADC Value) to reach for Metering valve (0-1023)
int CV_Metering_Valve; // Current Value (ADC Value) of Metering valve (0-1023)

byte digital_out_status; // Digital output (relay board) register

unsigned int timeout_counter = 20000; // Simple variable counter for monitoring timeout during communication
unsigned int PID_PV, PID_SV, PID_STATUS; // PID variables

unsigned int i, counter; // useful indexes for cycles and counters

// GLOBAL STRINGS -------------------------------------------
String inString = ""; // for incoming serial string data

// GLOBAL FLAGS
bool motor_flag = 0; // flag for activating motor control

// FUNCTIONS ------------------------------------------------
char convertCharToHex(char ch);


// EEPROM MEMORY ADDRESSES
// 0x00 MSB ADC_val
// 0x01 LSB ADC_val
// 0x02	Digital Output (relay board) status --> copy of digital_out_status register

// Software Serial for RS-485 Communication
SoftwareSerial PIDRS485(12, 13); // rx, tx

// ARDUINO SETUP
void setup() {
    // Digital Pins setup
	// Stepper motor **********
    pinMode(STEP_PIN1, OUTPUT);
    pinMode(STEP_PIN2, OUTPUT);
    pinMode(STEP_PIN3, OUTPUT);
    pinMode(STEP_PIN4, OUTPUT);
    // Relays *****************
    pinMode(RELAY_PIN1, OUTPUT);
    pinMode(RELAY_PIN2, OUTPUT);
    pinMode(RELAY_PIN3, OUTPUT);
    pinMode(RELAY_PIN4, OUTPUT);

    // Default serial port for PC communication setup
    Serial.begin(9600);

    // Set data rate for SoftwareSerial port (PIDRS485) compatible with standard
    // Arduino "SERIAL_8N1 (the default)"
    //
    // Configure OMEGA PID 7833 communication as follows:
    // CoSH: ON
    // C-SL: ASCII
    // C-no: 1
    // LEn: 8
    // PrtY: none
    // StoP: 1
    PIDRS485.begin(9600);
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
			case 'A': // Read Potentiometer ADC ************************************
			      ADC_val0 = analogRead(A0);
			      Serial.print("Pot ADC Value: ");
			      Serial.println(ADC_val0);
				break;
			case 'L': // Read LM35 temperrature with ADC ************************************
			      ADC_val1 = analogRead(A1);
			      Serial.print("LM35 ADC Value: ");
			      Serial.println(ADC_val1);
				break;
			case 'M': // Metering Valve Motor Control ************************************
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
			case 'S': // Solenoid Valve Control ************************************
				inString.setCharAt(0, '0');
				digital_out_status = inString.toInt();
						if (digital_out_status <= 15 && digital_out_status >= 0){
							Serial.println(digital_out_status);
						}
				break;
			case 'X': // Motor Stopped ************************************
				Serial.println("Motor Valve Stopped...");
				break;
			case 'Y': // read PID SV
				PIDRS485.flush(); // flush read register
				PIDRS485.println(":010310010001EA");
				inString = "                "; // prepare an empty string
				delay(50);
				i = 0;
				while (PIDRS485.available()>0){ // read RS485 input string
						inString.setCharAt(i, PIDRS485.read());
						i++;
						}
				inString.trim(); // remove white spaces
				// calculate SV, i.e. elements [8,9,10]
				if (i==0){
					Serial.println("PID connection timeout");
					}
				else {
					PID_SV = convertCharToHex(inString.charAt(8))*256+convertCharToHex(inString.charAt(9))*16+convertCharToHex(inString.charAt(10));
					Serial.print("SV: ");
					Serial.println(PID_SV);
					}
				break;
			case 'U': // read PID PV
				PIDRS485.flush(); // flush read register
				PIDRS485.println(":010310000001EB");
				inString = "                "; // prepare an empty string
				delay(50);
				i = 0;
				while (PIDRS485.available()>0){ // read RS485 input string
						inString.setCharAt(i, PIDRS485.read());
						i++;
						}
				inString.trim(); // remove white spaces
				// calculate SV, i.e. elements [8,9,10]
				if (i==0){
					Serial.println("PID connection timeout");
					}
				else {
					PID_PV = convertCharToHex(inString.charAt(8))*(256)+convertCharToHex(inString.charAt(9))*16+convertCharToHex(inString.charAt(10));
					Serial.print("PV: ");
					Serial.println(PID_PV);
					}
				break;
			case 'I': // read PID status (ON/OFF)
				PIDRS485.flush(); // flush read register
				PIDRS485.println(":010208140001E0");
				inString = "                "; // prepare an empty string
				delay(50);
				i = 0;
				while (PIDRS485.available()>0){ // read RS485 input string
						inString.setCharAt(i, PIDRS485.read());
						i++;
						}
				inString.trim(); // remove white spaces
				if (i==0){
					Serial.println("PID connection timeout");
					}
				else {
					if(inString.charAt(8)=='1'){
						Serial.println("PID ON");
						}
					else {
						Serial.println("PID OFF");
						}
					}
				break;
			case 'O': // PID ON
				PIDRS485.println(":01050814FF00DF");
				break;
			case 'P': // PID OFF
				PIDRS485.println(":010508140000DE");
				break;
			case 'Q': // Vaporizer Query
				Serial.println("*"); //Just prompt a "*" to say that vaporizer is connected
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
		if (SV_Metering_Valve < CV_Metering_Valve){
			// CW motor turn
			while (CV_Metering_Valve > SV_Metering_Valve){
				for(int i = 7; i>-1; i--)
				{
					Step(i);
					if (CV_Metering_Valve - SV_Metering_Valve <5){
						delay(10);
					}
				}
				CV_Metering_Valve = analogRead(A0); // Get current ADC value

				while (Serial.available()) { // STOP TURNING COMMAND!
				if (Serial.available() >0) {
					char c = Serial.read();
					inString += c; // Read data
					}
				}
				if (inString.length()> 0) {
						char CMD = inString.charAt(0);
						if (CMD=='X'){
							Step(Mot_OFF);
							break;
						}
						}

			}
		}
		if (SV_Metering_Valve > CV_Metering_Valve){
			// CCW motor turn
			while (CV_Metering_Valve < SV_Metering_Valve){
				for(int i = 0; i<8; i++)
				{
					Step(i);
					if (SV_Metering_Valve - CV_Metering_Valve <5){
						delay(10);
					}
				}
				CV_Metering_Valve = analogRead(A0); // Get current ADC value

				while (Serial.available()) { // STOP TURNING COMMAND!
				if (Serial.available() >0) {
					char c = Serial.read();
					inString += c; // Read data
					}
				}
				if (inString.length()> 0) {
						char CMD = inString.charAt(0);
						if (CMD=='X'){
							Step(Mot_OFF);
							break;
						}
						}
			}
		}
		Serial.println("*");
	}
}


char convertCharToHex(char ch)
/*
https://arduino.stackexchange.com/questions/39861/how-to-convert-an-hex-string-to-an-array-of-bytes
goddland_16 answer
 */
{
  char returnType;
  switch(ch)
  {
    case '0':
    returnType = 0;
    break;
    case  '1' :
    returnType = 1;
    break;
    case  '2':
    returnType = 2;
    break;
    case  '3':
    returnType = 3;
    break;
    case  '4' :
    returnType = 4;
    break;
    case  '5':
    returnType = 5;
    break;
    case  '6':
    returnType = 6;
    break;
    case  '7':
    returnType = 7;
    break;
    case  '8':
    returnType = 8;
    break;
    case  '9':
    returnType = 9;
    break;
    case  'A':
    returnType = 10;
    break;
    case  'B':
    returnType = 11;
    break;
    case  'C':
    returnType = 12;
    break;
    case  'D':
    returnType = 13;
    break;
    case  'E':
    returnType = 14;
    break;
    case  'F' :
    returnType = 15;
    break;
    default:
    returnType = 0;
    break;
  }
  return returnType;
}
