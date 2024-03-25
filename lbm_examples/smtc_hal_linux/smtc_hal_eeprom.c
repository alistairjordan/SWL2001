/*!
 * \file      smtc_hal_eeprom.c
 *
 * \brief     EEPROM Hardware Abstraction Layer implementation
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
#include <stdlib.h>
#include <stdint.h>   // C99 types
#include <stdbool.h>  // bool type
#include <stdio.h>    // TODO: check if needed
#

#include "smtc_hal_eeprom.h"
#include "smtc_hal_mcu.h"

#include <string.h>
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

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

void hal_eeprom_write_buffer( uint32_t addr, const uint8_t* buffer, uint32_t size )
{
    //open eeprom emulation file
    FILE *fp;
    fp=fopen("eeprom.dat","rb+");
    if(!fp) 
    {
        printf("EEPROM Error: File cannot be opened for write\n") ;
        exit(-1);
    }

    if (fseek(fp, addr, SEEK_SET) != 0) {
        printf("EEPROM Error: Cannot access address in EEPROM\n");
        exit(-1);
    }

    fwrite(buffer, size, 1, fp);

    fclose(fp);
}

void hal_eeprom_read_buffer( uint32_t addr, uint8_t* buffer, uint32_t size )
{
    //open eeprom emulation file
    FILE *fp;
    fp=fopen("eeprom.dat","rb+");
    if(!fp) 
    {
        printf("EEPROM Error: File cannot be opened for read\n") ;
        exit(-1);
    }

    if (fseek(fp, addr, SEEK_SET) != 0) {
        printf("EEPROM Error: Cannot access address in EEPROM\n");
        exit(-1);
    }

    fread(buffer, size, 1, fp);
    
    fclose(fp);
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

/* --- EOF ------------------------------------------------------------------ */
