/*!
 * \file      smtc_hal_lp_timer.c
 *
 * \brief     Implements Low Power Timer utilities functions.
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */
#include <stdio.h>
#include <stdint.h>   // C99 types
#include <stdbool.h>  // bool type
#include <time.h>
#include <errno.h>  
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include "smtc_hal_lp_timer.h"
//#include "stm32l4xx_hal.h"
#include "smtc_hal_mcu.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

#define HAL_LP_TIMER_NB 2  //!< Number of supported low power timers

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */
 pthread_t thread_id[HAL_LP_TIMER_NB];
 bool thread_running[HAL_LP_TIMER_NB] = {false,false};

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */
int msleep(long msec);

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

void hal_lp_timer_init( hal_lp_timer_id_t id )
{
    // Handled in Linux
}

/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}


void* hal_lp_timer_start_thread(void* lp_timer_start_context_void) {
    hal_lp_timer_irq_t* tmr_irq = (hal_lp_timer_irq_t*)lp_timer_start_context_void;
    printf("I AM HERE");
    fflush(stdout);
    msleep((long)tmr_irq->ms);
    (tmr_irq->callback)(tmr_irq->context);
//hal_gpio_irq_t* irq = (hal_gpio_irq_t*)irq_void;
    return tmr_irq;
}
void hal_lp_timer_start( hal_lp_timer_id_t id, const uint32_t milliseconds, hal_lp_timer_irq_t* tmr_irq )
{
    printf("I AM HERE BRO");
    fflush(stdout);
    printf("INFO:hal_lp_timer_start\n");
    fflush(stdout);
    tmr_irq->ms = milliseconds;
    pthread_create(&thread_id[id],NULL,hal_lp_timer_start_thread,tmr_irq);
    thread_running[id]=true;
    //pthread_create(&thread_id[irq->pin],NULL,irq_handler,irq);
}

void hal_lp_timer_stop( hal_lp_timer_id_t id )
{
    if (id > sizeof(thread_id)) {
        return;
    }
    if (!thread_running[id]) {
        return;
    }
    // Handled in Linux
    //pthread_kill(thread_id[irq->pin],0);
    pthread_kill(thread_id[id],0);
}

void hal_lp_timer_irq_enable( hal_lp_timer_id_t id )
{
    // Handled in Linux
}

void hal_lp_timer_irq_disable( hal_lp_timer_id_t id )
{
   // Handled in Linux
}
/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

/* --- EOF ------------------------------------------------------------------ */
