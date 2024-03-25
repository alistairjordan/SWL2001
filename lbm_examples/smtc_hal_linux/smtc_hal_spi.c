/*!
 * \file      smtc_hal_spi.c
 *
 * \brief     SPI Hardware Abstraction Layer implementation
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

#include <stdint.h>   // C99 types
#include <stdbool.h>  // bool type
#include <string.h>

#include "smtc_hal_spi.h"
// #include "stm32l4xx_hal.h"
// #include "stm32l4xx_ll_spi.h"
#include "spidev_lib/spidev_lib.h"
#include "smtc_hal_gpio.h"

#include "modem_pinout.h"
#include "smtc_hal_mcu.h"

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

typedef struct spi_s
{
    uint8_t      interface;
    uint8_t      handle;
    struct
    {
        hal_gpio_pin_names_t mosi;
        hal_gpio_pin_names_t miso;
        hal_gpio_pin_names_t sclk;
    } pins;
    int fd;
} spi_t;

#define SPI_FILE "/dev/spidev0.0"

spi_config_t spi_config;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

enum SPI_INTERFACES {
    SPI0,
    SPI1,
    SPI2
};

static spi_t spi_periph[] = {
    [0] =
        {
            .interface = SPI0,
            .handle    = 0,
            .pins =
                {
                    .mosi = NC,
                    .miso = NC,
                    .sclk = NC,
                },
            .fd = -1,
        },
};

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

void hal_spi_init( const uint32_t id, const hal_gpio_pin_names_t mosi, const hal_gpio_pin_names_t miso,
                   const hal_gpio_pin_names_t sclk )
{
    //assert_param( ( id > 0 ) && ( ( id - 1 ) < sizeof( spi_periph ) ) );
    uint32_t local_id = id - 1;

    spi_config.mode=0;
    spi_config.speed=1000000;
    spi_config.delay=0;
    spi_config.bits_per_word=8;
    spi_periph[local_id].fd = spi_open(SPI_FILE,spi_config);
}

void hal_spi_de_init( const uint32_t id )
{
    //assert_param( ( id > 0 ) && ( ( id - 1 ) < sizeof( spi_periph ) ) );
    uint32_t local_id = id - 1;

    spi_close(spi_periph[local_id].fd);
}

uint16_t hal_spi_in_out( const uint32_t id, const uint16_t out_data )
{
    //assert_param( ( id > 0 ) && ( ( id - 1 ) < sizeof( spi_periph ) ) );
    uint32_t local_id = id - 1;

    uint8_t tx_buffer[1] = {(uint8_t)out_data};
    uint8_t rx_buffer[1];
    spi_xfer(spi_periph[local_id].fd,tx_buffer,1,rx_buffer,1);
    printf("tx: %x, rx: %x\t",tx_buffer[0],rx_buffer[0]);
    return( rx_buffer[0] ); //hmmmmmmmmmmmm this isn't quite 16bits word size........
}
/* --- EOF ------------------------------------------------------------------ */
