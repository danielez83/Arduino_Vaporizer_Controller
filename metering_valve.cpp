/*
 * Metering valve stepper motor functions
 *
 *  Created on: 2 set 2019
 *      Author: Utente
 */

#include <Arduino.h>
#include "metering_valve.h"

// Set the state of digital output register (see pins definition in header file)
void Output_state( int i4,int i3,int i2,int i1)
{
  if (i1==1) digitalWrite(STEP_PIN1,HIGH); else digitalWrite(STEP_PIN1,LOW);
  if (i2==1) digitalWrite(STEP_PIN2,HIGH); else digitalWrite(STEP_PIN2,LOW);
  if (i3==1) digitalWrite(STEP_PIN3,HIGH); else digitalWrite(STEP_PIN3,LOW);
  if (i4==1) digitalWrite(STEP_PIN4,HIGH); else digitalWrite(STEP_PIN4,LOW);
}

// Do a single step (a step is a sequence of 0:7, minimum time resolution is 1 ms)
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
