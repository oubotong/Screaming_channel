/**
 * Copyright (c) 2014 - 2017, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
*
* @defgroup nrf_radio_test_example_main main.c
* @{
* @ingroup nrf_radio_test_example
* @brief Radio Test Example Application main file.
*
* This file contains the source code for a sample application using the NRF_RADIO, and is controlled through the serial port.
*
*/

#include <zephyr.h>
#include <device.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "nrf.h"
#include "radio_test.h"
#include <unistd.h>



/*
 * Poor man's hexdump
 *
 * Only for byte arrays! We sleep a bit for longer arrays to make sure that
 * we're not faster than the serial port. Also, newlines every now and then seem
 * to be necessary to flush a buffer somewhere.
 */
#define hexdump(byte_arr)                               \
    do {                                                \
        printf("(%u bytes) ", sizeof(byte_arr));        \
        for (size_t i = 0; i < sizeof(byte_arr); ++i) { \
            if (i % 32 == 0) {                          \
                printf("\r\n");                         \
            }                                           \
                                                        \
            printf("%02x", byte_arr[i]);                \
        }                                               \
    } while (0)

static uint8_t mode_          = RADIO_MODE_MODE_Ble_1Mbit;
static uint8_t txpower_       = RADIO_TXPOWER_TXPOWER_0dBm;
static int channel_start_     = 0;
static int channel_end_       = 80;
static int delayms_           = 10;
//static uint8_t rcounter_      = 0;

typedef enum
{
    RADIO_TEST_NOP,             /**< No test running.                                */
    RADIO_TEST_TXCC,            /**< TX constant carrier.                            */
    RADIO_TEST_TXMC,            /**< TX modulated carrier.                           */
    RADIO_TEST_TXSWEEP,         /**< TX sweep.                                       */
    RADIO_TEST_RXC,             /**< RX constant carrier.                            */
    RADIO_TEST_RXSWEEP,         /**< RX sweep.                                       */
    RADIO_TEST_NOISYOP,         /**< Some noisy op.                                  */
} radio_tests_t;

#define BELL 7 // Bell

/** @brief Function for configuring all peripherals used in this example.
*/
static void init(void)
{
    NRF_RNG_NS->TASKS_START = 1;

    // Start 16 MHz crystal oscillator
    NRF_CLOCK_NS->EVENTS_HFCLKSTARTED  = 0;
    NRF_CLOCK_NS->TASKS_HFCLKSTART     = 1;

    // Wait for the external oscillator to start up
    while (NRF_CLOCK_NS->EVENTS_HFCLKSTARTED == 0)
    {
        // Do nothing.
    }
}


/** @brief Function for outputting usage info to the serial port.
*/
static void help(void)
{
    printf("Usage:\r\n");
    printf("a: Enter start channel for sweep/channel for constant carrier\r\n");
    printf("b: Enter end channel for sweep\r\n");
    printf("c: Start TX carrier\r\n");
    printf("d: Enter time on each channel (1ms-99ms)\r\n");
    printf("e: Cancel sweep/carrier\r\n");
    printf("l: Test the crypto hardware\r\n");

    printf("m: Enter data rate\r\n");
    printf("o: Start modulated TX carrier\r\n");
    printf("p: Enter output power\r\n");
    printf("q: Start modulated TX carrier with encryption\r\n");
    printf("s: Print current delay, channels and so on\r\n");
    printf("r: Start RX sweep\r\n");
    printf("t: Start TX sweep\r\n");
    printf("x: Start RX carrier\r\n");
    printf("y: Start noisy operation\r\n");
    printf("z: End noisy operation\r\n");

    printf("v: Enter simplified power analysis mode\r\n");
    printf("   m: Enter switching mask\r\n");
    printf("   s: Switch\r\n");
    printf("   q: Quit power analysis mode\r\n");

}


/** @brief Function for reading the data rate.
*/
void get_datarate(void)
{
    uint8_t c;

#ifndef NRF52840_XXAA
    printf("Enter data rate ('0'=250 Kbit/s, '1'=1 Mbit/s, '2'=2 Mbit/s and '3'=BLE 1 Mbit/s):\r\n");
#else
    printf("Enter data rate ('1'=1 Mbit/s, '2'=2 Mbit/s and '3'=BLE 1 Mbit/s):\r\n");
#endif //NRF52840_XXAA
    while (true)
    {
        scanf("%c",&c);
        if ((c >= '0') && (c <= '2'))
        {
            printf("%c\r\n",c);
            break;
        }
        else
        {
            printf("%c\r\n",BELL);
        }
    }

    if (c == '1')
    {
        mode_ = RADIO_MODE_MODE_Nrf_1Mbit;
    }
    else if (c == '2')
    {
        mode_ = RADIO_MODE_MODE_Nrf_2Mbit;
    }
    else if (c == '3')
    {
        mode_ = RADIO_MODE_MODE_Ble_1Mbit;
    }
    printf("\r\n");
}


/** @brief Function for reading the output power.
*/
void get_power(void)
{
    uint8_t c;

    printf("Enter output power ('0'=+4 dBm, '1'=0 dBm,...,'7'=-30 dBm):\r\n");
    c=1;

    switch (c)
    {

        case '1':
            txpower_ =  RADIO_TXPOWER_TXPOWER_0dBm;
            break;

        case '2':
            txpower_ = RADIO_TXPOWER_TXPOWER_Neg4dBm;
            break;

        case '3':
            txpower_ = RADIO_TXPOWER_TXPOWER_Neg8dBm;
            break;

        case '4':
            txpower_ = RADIO_TXPOWER_TXPOWER_Neg12dBm;
            break;

        case '5':
            txpower_ = RADIO_TXPOWER_TXPOWER_Neg16dBm;
            break;

        case '6':
            txpower_ = RADIO_TXPOWER_TXPOWER_Neg20dBm;
            break;

        case '7':
            // fall through

        default:
            txpower_ = RADIO_TXPOWER_TXPOWER_Neg30dBm;
            break;
    }
    printf("\r\n");
}


/** @brief Function for printing parameters to the serial port.
*/
void print_parameters(void)
{
    printf("Parameters:\r\n");
    switch (mode_)
    {

        case RADIO_MODE_MODE_Nrf_1Mbit:
            printf("Data rate...........: 1 Mbit/s\r\n");
            break;

        case RADIO_MODE_MODE_Nrf_2Mbit:
            printf("Data rate...........: 2 Mbit/s\r\n");
            break;

        case RADIO_MODE_MODE_Ble_1Mbit:
            printf("Data rate...........: BLE 1 Mbit/s\r\n");
            break;
    }

    switch (txpower_)
    {

        case RADIO_TXPOWER_TXPOWER_0dBm:
            printf("TX Power............: 0 dBm\r\n");
            break;

        case RADIO_TXPOWER_TXPOWER_Neg4dBm:
            printf("TX Power............: -4 dBm\r\n");
            break;

        case RADIO_TXPOWER_TXPOWER_Neg8dBm:
            printf("TX Power............: -8 dBm\r\n");
            break;

        case RADIO_TXPOWER_TXPOWER_Neg12dBm:
            printf("TX Power............: -12 dBm\r\n");
            break;

        case RADIO_TXPOWER_TXPOWER_Neg16dBm:
            printf("TX Power............: -16 dBm\r\n");
            break;

        case RADIO_TXPOWER_TXPOWER_Neg20dBm:
            printf("TX Power............: -20 dBm\r\n");
            break;

        case RADIO_TXPOWER_TXPOWER_Neg30dBm:
            printf("TX Power............: -30 dBm\r\n");
            break;

        default:
            // No implementation needed.
            break;

    }
    printf("(Start) Channel.....: %d\r\n",channel_start_);
    printf("End Channel.........: %d\r\n",channel_end_);
    printf("Time on each channel: %d ms\r\n",delayms_);
    printf("Low frequency clock.: %s\r\n",
           (NRF_CLOCK_NS->LFCLKSTAT & CLOCK_LFCLKSTAT_STATE_Msk) ? "on" : "off");
}

bool timer1_init()
{
    NRF_TIMER1_NS->TASKS_STOP  = 1;
    NRF_TIMER1_NS->TASKS_CLEAR = 1;
    NRF_TIMER1_NS->INTENCLR    = 0xffffffff;
    NRF_TIMER1_NS->INTENSET    = TIMER_INTENSET_COMPARE0_Msk;

    NRF_TIMER1_NS->SHORTS    = (1 << TIMER_SHORTS_COMPARE0_CLEAR_Pos);
    NRF_TIMER1_NS->PRESCALER = 22; // f_timer = 16MHz/(2^prescalar)

    NRF_TIMER1_NS->CC[0] = 0;

    NVIC_ClearPendingIRQ(TIMER1_IRQn);
    NVIC_EnableIRQ(TIMER1_IRQn);
    return true;
}

void start_noisy_op(void)
{
    NRF_TIMER1_NS->TASKS_CLEAR = 1;
    NRF_TIMER1_NS->TASKS_START = 1;
}

void stop_noisy_op(void)
{
    NRF_TIMER1_NS->TASKS_STOP = 1;
}

/*
 * @brief Function to read 16 integers from the serial line and to write them in
 * to a bytearray
 */
void read_128(uint8_t* in){
    int tmp;
    for(int i=0;i<16;i++){
        scanf("%d",&tmp);
        in[i] = (uint8_t)tmp;
    }
    /*scanf("%hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd",*/
           /*&in[ 0],&in[ 1],&in[ 2],&in[ 3],*/
           /*&in[ 4],&in[ 5],&in[ 6],&in[ 7],*/
           /*&in[ 8],&in[ 9],&in[10],&in[11],*/
           /*&in[12],&in[13],&in[14],&in[15]);*/
}

/*
 * @brief Function to write a byte array as 16 integers to the serial line
 */
void write_128(uint8_t* out){
    printf("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\r\n",
           out[ 0],out[ 1],out[ 2],out[ 3],
           out[ 4],out[ 5],out[ 6],out[ 7],
           out[ 8],out[ 9],out[10],out[11],
           out[12],out[13],out[14],out[15]);
}

/*
 * brief Function that creates a preable for easy alignment
 */
void preamble(){
    typedef uint8_t state_t[4][4];
    state_t state = {0};
    uint8_t i,j;
    for(i=0;i<4;++i)
    {
      for(j = 0; j < 4; ++j)
      {
        state[i][j] ^= 0xff;
      }
    }
    for(i=0;i<4;++i)
    {
      for(j = 0; j < 4; ++j)
      {
        state[i][j] ^= 0xff;
      }
    }
    for(i=0;i<4;++i)
    {
      for(j = 0; j < 4; ++j)
      {
        state[i][j] ^= 0xff;
      }
    }
    for(i=0;i<4;++i)
    {
      for(j = 0; j < 4; ++j)
      {
        state[i][j] ^= 0xff;
      }
    }
    for(i=0;i<4;++i)
    {
      for(j = 0; j < 4; ++j)
      {
        state[i][j] ^= 0xff;
      }
    }
}


/*
 * @brief Function to handle power analysis
 */
void power_analysis_mode(){
    printf("Entering power analysis mode\r\n");
    uint8_t control;
    bool exit = false;
    uint8_t switching_mask[16] = {0};
    while(!exit){
        scanf("%c",&control);
        switch(control){
            case 'p':
                read_128(switching_mask);
                write_128(switching_mask); // dbg
                break;
            case 'q':
                exit = true;
                break;
            default:
                break;
        }
    }
    printf("Exiting power analysis mode\r\n");
}

typedef struct {
    bool sweep;
    bool ccm;
} main_state_t;

/** @brief Helper to switch off all activity.
 */
void all_off(main_state_t* state)
{
    if (state->sweep)
    {
        radio_sweep_end();
        state->sweep = false;
    }
    if (state->ccm)
    {
        state->ccm = false;
    }
}

/** @brief Function for main application entry.
 */
int main(void)
{
    uint32_t err_code;
    radio_tests_t test     = RADIO_TEST_NOP;
    radio_tests_t cur_test = RADIO_TEST_NOP;
    main_state_t state     = {false, false};

    init();
    //bsp_board_leds_init();
    printf("RF Test\r\n");
    NVIC_EnableIRQ(TIMER0_IRQn);
    NVIC_EnableIRQ(TIMER1_IRQn);
    __enable_irq();
    uint8_t control;
    channel_start_=0;
    test = cur_test;
    txpower_ =  RADIO_TXPOWER_TXPOWER_0dBm;
    test = RADIO_TEST_TXCC;
    all_off(&state);
    radio_tx_carrier(txpower_, mode_, channel_start_);
    //k_sleep(K_MSEC(10000));
    //bitcount_main();
        /*switch (control)
        {
            case 'a':
                while (true)
                {
                    printf("Enter start channel (two decimal digits, 00 to 80):\r\n");
                    channel_start_=0;
                    if ((channel_start_ <= 80)&&(channel_start_ >= 0))
                    {
                        printf("%d\r\n", channel_start_);
                        break;
                    }

                    printf("Channel must be between 0 and 80\r\n");
                }
                test = cur_test;
                
                break;

            case 'b':
                while (true)
                {
                    printf("Enter end channel (two decimal digits, 00 to 80):\r\n");
                    scanf("%d",&channel_end_);
                    if ((channel_end_ <= 80)&&(channel_start_ >= 0))
                    {
                        printf("%d\r\n", channel_end_);
                        break;
                    }
                    printf("Channel must be between 0 and 80\r\n");
                }
                test = cur_test;
                break;

            case 'c':
                test = RADIO_TEST_TXCC;
                break;

            case 'd':
                while (true)
                {
                    printf("Enter delay in ms (two decimal digits, 01 to 99):\r\n");
                    scanf("%d",&delayms_);
                    if ((delayms_ > 0) && (delayms_ < 100))
                    {
                        printf("%d\r\n", delayms_);
                        break;
                    }
                    printf("Delay must be between 1 and 99\r\n");
                }
                test = cur_test;
                break;

            case 'e':
                radio_sweep_end();
                cur_test = RADIO_TEST_NOP;
                break;

            case 'm':
                get_datarate();
                test = cur_test;
                break;

            case 'o':
                test = RADIO_TEST_TXMC;
                printf("TX modulated carrier\r\n");
                break;

            case 'p':
                get_power();
                test = cur_test;
                break;

            case 'r':
                test = RADIO_TEST_RXSWEEP;
                printf("RX Sweep\r\n");
                break;

            case 's':
                print_parameters();
                break;

            case 't':
                test = RADIO_TEST_TXSWEEP;
                printf("TX Sweep\r\n");
                break;

            case 'v':
                power_analysis_mode();
                break;

            case 'x':
                test = RADIO_TEST_RXC;
                printf("RX constant carrier\r\n");
                break;

            case 'y':
                test = RADIO_TEST_NOISYOP;
                printf("Exercise the processor\r\n");
                break;

            case 'z':
                stop_noisy_op();
                test = RADIO_TEST_NOP;
                printf("Relax the processor\r\n");
                break;

            case 'h':
                help();
                break;

            default:
                
                break;
        }*/

        /*switch (test)
        {
            case RADIO_TEST_TXCC:
                all_off(&state);
                radio_tx_carrier(txpower_, mode_, channel_start_);
                cur_test = test;
                test     = RADIO_TEST_NOP;
                break;

            case RADIO_TEST_TXMC:
                all_off(&state);
                radio_modulated_tx_carrier(txpower_, mode_, channel_start_);
                cur_test = test;
                test     = RADIO_TEST_NOP;
                break;

            case RADIO_TEST_TXSWEEP:
                all_off(&state);
                radio_tx_sweep_start(txpower_, mode_, channel_start_, channel_end_, delayms_);
                state.sweep = true;
                cur_test = test;
                test     = RADIO_TEST_NOP;
                break;

            case RADIO_TEST_RXC:
                all_off(&state);
                radio_rx_carrier(mode_, channel_start_);
                cur_test = test;
                test     = RADIO_TEST_NOP;
                break;

            case RADIO_TEST_RXSWEEP:
                radio_rx_sweep_start(mode_, channel_start_, channel_end_, delayms_);
                state.sweep = true;
                cur_test = test;
                test     = RADIO_TEST_NOP;
                break;

            case RADIO_TEST_NOISYOP:
                timer1_init();
                start_noisy_op();
                state.sweep    = false;
                cur_test = test;
                test     = RADIO_TEST_NOP;
                break;

            case RADIO_TEST_NOP:
            default:
                break;
        }*/
    //}
}

/** @} */
