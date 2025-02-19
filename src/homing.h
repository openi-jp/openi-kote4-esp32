#ifndef HOMING_H
#define HOMING_H

#include <Arduino.h>
#include <FastAccelStepper.h>

#include "openi_in.h"

extern FastAccelStepper *stacker_stepper1;
extern FastAccelStepper *stacker_stepper2;
extern FastAccelStepper *stacker_arm1;
extern FastAccelStepper *stacker_stepper4;

extern openi_in in27;

void homing();

#endif
