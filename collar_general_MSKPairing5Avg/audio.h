/*
 * audio.h
 *
 *  Created on: Mar 14, 2021
 *      Author: Jonathan Chen
 */

#ifndef AUDIO_H_
#define AUDIO_H_

#include "driverlib.h"
#include "Board.h"


/**
 * Setup pins for DAC audio output and SHUTDOWNn pin
 */
void configure_audio_pins();

/**
 * Initialize audio indicator, turning on DAC and audio amplifier
 */
void init_audio();

/**
 * Setup timers to play a 1 kHz pulse for a certain number of sample lengths
 *
 * @param cycles    Number of times to play sample
 */
void play_1kHz_pulse(int cycles);

/**
 * Setup timers to play a 2 kHz pulse for a certain number of sample lengths
 *
 * @param cycles    Number of times to play sample
 */
void play_2kHz_pulse(int cycles);

/**
 * Setup timers to play a 3 kHz pulse for a certain number of sample lengths
 *
 * @param cycles    Number of times to play sample
 */
void play_3kHz_pulse(int cycles);


#endif /* AUDIO_H_ */
