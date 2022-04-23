/*
 * audio.c
 *
 *  Created on: Mar 14, 2021
 *      Author: Jonathan Chen
 */

#include <audio.h>

// Sine value arrays with varying amplitudes to determine volume
const unsigned int sine_value[] = {2048,2226,2403,2577,     // starting sine wave sample
                      2748,2913,3071,3222,
                      3363,3495,3616,3724,
                      3820,3903,3971,4025,
                      4063,4087,4095,4087,
                      4063,4025,3971,3903,
                      3820,3724,3616,3495,
                      3363,3222,3071,2913,
                      2748,2577,2403,2226,
                      2048,1869,1692,1518,
                      1347,1182,1024,873,
                      732,600,479,371,
                      275,192,124,70,
                      32,8,1,8,
                      32,70,124,192,
                      275,371,479,600,
                      732,873,1024,1182,
                      1347,1518,1692,1869};
const unsigned int sine_value20pzero[] = {0,1,6,14,24,      // starts and ends at 0 to help prevent clipping
                                          38,55,74,95,
                                          120,146,174,
                                          204,236,269,
                                          303,338,373,
                                          409,445,480,
                                          515,549,582,
                                          614,644,672,
                                          699,723,744,
                                          764,780,794,
                                          805,812,817,
                                          819,817,812,
                                          805,794,780,
                                          764,744,723,
                                          699,672,644,
                                          614,582,549,
                                          515,480,445,
                                          409,373,338,
                                          303,269,236,
                                          204,174,146,
                                          120,95,74,55,
                                          38,24,14,6,0};
const unsigned int sine_value50pzero[] = {4,16,35,62,96,
                                          137,185,239,300,
                                          366,436,512,591,
                                          673,759,846,934,
                                          1024,1113,1201,1288,
                                          1374,1456,1535,1611,
                                          1681,1747,1808,1862,
                                          1910,1951,1985,2012,
                                          2031,2043,2047,2043,
                                          2031,2012,1985,1951,
                                          1910,1862,1808,1747,
                                          1681,1611,1535,1456,
                                          1374,1288,1201,1113,
                                          1024,934,846,759,
                                          673,591,512,436,
                                          366,300,239,185,
                                          137,96,62,35,
                                          16,4,0};
const unsigned int sine_value90pzero[] = {7,28,63,112,
                                          173,247,334,431,
                                          540,659,786,922,
                                          1064,1213,1366,1523,
                                          1682,1843,2003,2163,
                                          2320,2473,2621,2764,
                                          2899,3027,3145,3254,
                                          3352,3438,3512,3574,
                                          3622,3657,3678,3685,
                                          3678,3657,3622,3574,
                                          3512,3438,3352,3254,
                                          3145,3027,2899,2764,
                                          2621,2473,2320,2163,
                                          2003,1843,1682,1523,
                                          1366,1213,1064,922,
                                          786,659,540,431,
                                          334,247,173,112,
                                          63,28,7,0};
const unsigned int sine_value100pzero[] = {8,
                                           32,70,124,192,
                                           275,371,479,600,
                                           732,873,1024,1182,
                                           1347,1518,1692,1869,
                                           2048,2226,2403,2577,
                                           2748,2913,3071,3222,
                                           3363,3495,3616,3724,
                                           3820,3903,3971,4025,
                                           4063,4087,4095,4087,
                                           4063,4025,3971,3903,
                                           3820,3724,3616,3495,
                                           3363,3222,3071,2913,
                                           2748,2577,2403,2226,
                                           2048,1869,1692,1518,
                                           1347,1182,1024,873,
                                           732,600,479,371,
                                           275,192,124,70,
                                           32,8,0};


int size = sizeof sine_value / sizeof sine_value[0];
unsigned int cooldownCycles = 1400; // Number of cycles to rest to allow amplifier output to drift back to 0V

unsigned int counter = 0;
unsigned int sampleCycle = 0;
unsigned int numSamples = 100;      // number of times to play the sample, changes depending on play_#kHz_pulse  function called
bool playingSample = true;


/**
 * Setup pins for DAC audio output and SHUTDOWNn pin
 */
void configure_audio_pins() {
    //Configure OA0 functionality
    GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_P1,
            GPIO_PIN1,
            GPIO_TERNARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionInputPin(
            GPIO_PORT_P1,                           // Port 1
            GPIO_PIN3,                              // Pin 3
            GPIO_TERNARY_MODULE_FUNCTION);          // Op Amp noninverting pin

    //Configure Shutdown Pin for Amplifier
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN2);

    //Configure pin 5.0 to output timer B2.1
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P5, GPIO_PIN0, GPIO_PRIMARY_MODULE_FUNCTION);
}

/**
 * Initialize audio indicator, turning on DAC and audio amplifier
 */
void init_audio() {

    //Select DVCC as DAC reference voltage
    SAC_DAC_selectRefVoltage(SAC0_BASE, SAC_DAC_PRIMARY_REFERENCE);

    //Select the load trigger for DAC data latch
    //DAC always loads data from DACDAT at the positive edge of Timer output TB2.1
    SAC_DAC_selectLoad(SAC0_BASE, SAC_DAC_LOAD_DEVICE_SPECIFIC_0);

    //Write data to DAC Data Register SACxDAT
    //DAC_data is an unsigned int type variable defined by user SAC_DAC_setData(SAC0_BASE, DAC_data);
    uint16_t DAC_data = 0x0000;
    SAC_DAC_setData(SAC0_BASE, DAC_data);
    //Enable DAC
    SAC_DAC_enable(SAC0_BASE);

    //Select internal DAC for positive input and PGA source for negative input
    SAC_OA_init(SAC0_BASE, SAC_OA_POSITIVE_INPUT_SOURCE_DAC,
         SAC_OA_NEGATIVE_INPUT_SOURCE_PGA);

    //Select Buffer Mode
    SAC_PGA_setMode(SAC0_BASE, SAC_PGA_MODE_BUFFER);

    //Enable OA
    SAC_OA_enable(SAC0_BASE);

    //Enable SAC
    SAC_enable(SAC0_BASE);
}

/**
 * Setup timers to play a 1 kHz pulse for a certain amount of time
 *
 * @param cycles    Number of cycles to play the pulse for
 */
void play_1kHz_pulse(int cycles) {

    __disable_interrupt(); // Disable interrupt
    numSamples = cycles;   // Set number of cycles to play
    cooldownCycles = 1200;

    //Turn on Amplifier
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN2);
    //Configure and start timer
    Timer_B_initUpModeParam tParam = {0};
    tParam.clockSource = TIMER_B_CLOCKSOURCE_SMCLK;
    tParam.clockSourceDivider = TIMER_B_CLOCKSOURCE_DIVIDER_1;
    tParam.timerPeriod = 26;
    tParam.timerInterruptEnable_TBIE = TIMER_B_TBIE_INTERRUPT_DISABLE;
    tParam.captureCompareInterruptEnable_CCR0_CCIE = TIMER_B_CCIE_CCR0_INTERRUPT_ENABLE;
    tParam.timerClear = TIMER_B_DO_CLEAR;
    tParam.startTimer = true;
    Timer_B_initUpMode(TB2_BASE, &tParam);

    //Initialize compare mode to generate signal advance
    Timer_B_initCompareModeParam tparam1 = {0};
    tparam1.compareRegister = TIMER_B_CAPTURECOMPARE_REGISTER_1;
    tparam1.compareInterruptEnable = TIMER_B_CAPTURECOMPARE_INTERRUPT_DISABLE;
    tparam1.compareOutputMode = TIMER_B_OUTPUTMODE_TOGGLE_RESET;
    tparam1.compareValue = 13;
    Timer_B_initCompareMode(TB2_BASE, &tparam1);

    playingSample = true;
    __enable_interrupt(); // Reenable interrupt

}

/**
 * Setup timers to play a 1 kHz pulse for a certain amount of time
 *
 * @param cycles    Number of cycles to play the pulse for
 */
void play_2kHz_pulse(int cycles) {
    cooldownCycles = 2400;

    __disable_interrupt(); // Disable interrupt
    numSamples = cycles;   // Set number of cycles to play

    //Turn on Amplifier
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN2);
    //Configure and start timer
    Timer_B_initUpModeParam tParam = {0};
    tParam.clockSource = TIMER_B_CLOCKSOURCE_SMCLK;
    tParam.clockSourceDivider = TIMER_B_CLOCKSOURCE_DIVIDER_1;
    tParam.timerPeriod = 13;
    tParam.timerInterruptEnable_TBIE = TIMER_B_TBIE_INTERRUPT_DISABLE;
    tParam.captureCompareInterruptEnable_CCR0_CCIE = TIMER_B_CCIE_CCR0_INTERRUPT_ENABLE;
    tParam.timerClear = TIMER_B_DO_CLEAR;
    tParam.startTimer = true;
    Timer_B_initUpMode(TB2_BASE, &tParam);

    //Initialize compare mode to generate signal advance
    Timer_B_initCompareModeParam tparam1 = {0};
    tparam1.compareRegister = TIMER_B_CAPTURECOMPARE_REGISTER_1;
    tparam1.compareInterruptEnable = TIMER_B_CAPTURECOMPARE_INTERRUPT_DISABLE;
    tparam1.compareOutputMode = TIMER_B_OUTPUTMODE_TOGGLE_RESET;
    tparam1.compareValue = 6;
    Timer_B_initCompareMode(TB2_BASE, &tparam1);

    playingSample = true;
    __enable_interrupt(); // Reenable interrupt

}

/**
 * Setup timers to play a 1 kHz pulse for a certain amount of time
 *
 * @param cycles    Number of cycles to play the pulse for
 */
void play_3kHz_pulse(int cycles) {
    cooldownCycles = 2400;

    __disable_interrupt(); // Disable interrupt
    numSamples = cycles;   // Set number of cycles to play

    //Turn on Amplifier
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN2);
    //Configure and start timer
    Timer_B_initUpModeParam tParam = {0};
    tParam.clockSource = TIMER_B_CLOCKSOURCE_SMCLK;
    tParam.clockSourceDivider = TIMER_B_CLOCKSOURCE_DIVIDER_1;
    tParam.timerPeriod = 9;
    tParam.timerInterruptEnable_TBIE = TIMER_B_TBIE_INTERRUPT_DISABLE;
    tParam.captureCompareInterruptEnable_CCR0_CCIE = TIMER_B_CCIE_CCR0_INTERRUPT_ENABLE;
    tParam.timerClear = TIMER_B_DO_CLEAR;
    tParam.startTimer = true;
    Timer_B_initUpMode(TB2_BASE, &tParam);

    //Initialize compare mode to generate signal advance
    Timer_B_initCompareModeParam tparam1 = {0};
    tparam1.compareRegister = TIMER_B_CAPTURECOMPARE_REGISTER_1;
    tparam1.compareInterruptEnable = TIMER_B_CAPTURECOMPARE_INTERRUPT_DISABLE;
    tparam1.compareOutputMode = TIMER_B_OUTPUTMODE_TOGGLE_RESET;
    tparam1.compareValue = 4;
    Timer_B_initCompareMode(TB2_BASE, &tparam1);

    playingSample = true;
    __enable_interrupt(); // Reenable interrupt

}


//-- Timer ISR to iterate through a sample ---------------------------
#pragma vector = TIMER2_B0_VECTOR
__interrupt void ISR_TB2_CCR0(void)
{
    if (playingSample) {
        SAC_DAC_setData(SAC0_BASE, sine_value100pzero[counter]);
        counter += 1;
        if (counter >= size) {  // increment cycle, reset counter
            counter = 0;
            sampleCycle += 1;
            if (sampleCycle >= numSamples) { // switch off sample, reset counters
                counter = 0;
                sampleCycle = 0;
                playingSample = false;
            }
        }
    }
    else {
        counter += 1;   // cooldown loop to bring output back to zero, prevent clipping
        if (counter >= cooldownCycles) {
            counter = 0;
            GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN2); // shut down amplifier
            Timer_B_disableCaptureCompareInterrupt(TB2_BASE, TIMER_B_CAPTURECOMPARE_REGISTER_0);
        }
    }
}

