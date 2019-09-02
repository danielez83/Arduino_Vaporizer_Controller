/*
 * Functions and constants declaration for motorized metering valve
 * Valve: Swagelok SS-SS2 (1/8 in.)
 * Motor: 28byj-48
 * Driver: ULN2003AN
 * Created on: 2 set 2019
 * Author: Daniele Z.
 */

#ifndef METERING_VALVE_H_
#define METERING_VALVE_H_

// Constants and pin definition ****************

// Stepper motor pins
static const int  STEP_PIN1 = 8; // --> IN1 ULN2003AN board
static const int  STEP_PIN2 = 9; // --> IN2 ULN2003AN board
static const int  STEP_PIN3 = 10; // --> IN3 ULN2003AN board
static const int  STEP_PIN4 = 11; // --> IN4 ULN2003AN board
static const int Mot_OFF = 99; // Turn off stepper motor
static const int Mot_REV = 98; // Reverse stepper motor revolution

// Functions ***********************************
void Output_state( int i4,int i3,int i2,int i1);
void Step(int state);

#endif /* METERING_VALVE_H_ */
