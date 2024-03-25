/*!
 * \file      smtc_hal_gpio.c
 *
 * \brief     GPIO Hardware Abstraction Layer implementation
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
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>   // C99 types
#include <stdbool.h>  // bool type
#include <poll.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include "smtc_hal_gpio.h"

//#include "stm32l4xx_hal.h"

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

/*!
 * GPIO setup data structure
 */
typedef struct bsp_gpio_s
{
    hal_gpio_pin_names_t pin;
    uint32_t             mode;
    uint32_t             pull;
    uint32_t             speed;
    uint32_t             alternate;
} gpio_t;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

pthread_t thread_id[255];

/*!
 * Array holding attached IRQ gpio data context
 */
//static hal_gpio_irq_t const* gpio_irq[16];

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/*!
 * Generic gpio initialization
 *
 * \param [in/out] gpio  Holds MCU gpio parameters
 * \param [in]     value Initial MCU pit value
 * \param [in/out] irq   Pointer to IRQ data context.
 *                         NULL when setting gpio as output
 */
//static void gpio_init( const gpio_t* gpio, const uint32_t value, const hal_gpio_irq_t* irq );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

//
// MCU input pin Handling
//

void *irq_handler(void* irq_void) {
    hal_gpio_irq_t* irq = (hal_gpio_irq_t*)irq_void;
    int retval;
    char value_path[50];
    char readbuf[1];
    struct pollfd pfd;
    int read_bytes;

    snprintf(value_path, sizeof(value_path), "/sys/class/gpio/gpio%d/value", irq->pin);
    int value_file = open(value_path,O_RDONLY);
    if (value_file == -1) {
        printf("IRQ: Failed to open GPIO value file for %d", irq->pin);
        exit(-1);
    }
	pfd.fd = value_file;
	pfd.events = POLLERR | POLLPRI;
	pfd.revents = 0;
    while (1) {
        while ((retval = poll(&pfd, 1, 100)) >= 0) {
            if (pfd.revents & (POLLERR | POLLPRI)) {
                pfd.revents = 0;
                printf("IRQ: INTERUPT on %d", irq->pin);
                lseek(value_file, 0, SEEK_SET);
                read_bytes = read(value_file, readbuf, sizeof(readbuf));
                if (read_bytes < 0) {
                    printf("ERROR: Poll Read Failed\n");
                    exit(-1);
                }
                printf("%.*s", (int)read_bytes, readbuf);
                (irq->callback)(irq->context);
            }
        }
        usleep(10);
    }
}

void hal_gpio_init_in( const hal_gpio_pin_names_t pin, const hal_gpio_pull_mode_t pull_mode,
                       const hal_gpio_irq_mode_t irq_mode, hal_gpio_irq_t* irq )
{
    if (pin == NC) {
        return;
    }
    // Export Pin
    FILE *export_file = fopen("/sys/class/gpio/export", "w");
    if (export_file == NULL) {
        printf("Failed to open GPIO export file");
        exit(-1);
    }
    fprintf(export_file, "%d", pin);
    fclose(export_file);

    // Set direction
    char direction_path[50];
    snprintf(direction_path, sizeof(direction_path), "/sys/class/gpio/gpio%d/direction", pin);
    FILE *direction_file = fopen(direction_path, "w");
    if (direction_file == NULL) {
        printf("Failed to open GPIO direction file");
        exit(-1);
    }
    fprintf(direction_file, "in");
    fclose(direction_file);

    // Set pull mode (if required)
    if (pull_mode != BSP_GPIO_PULL_MODE_NONE) {
        char pull_path[50];
        snprintf(pull_path, sizeof(pull_path), "/sys/class/gpio/gpio%d/active_low", pin);
        FILE *pull_file = fopen(pull_path, "w");
        if (pull_file == NULL) {
            printf("Failed to open GPIO active_low file");
            exit(-1);
        }
        if (pull_mode == BSP_GPIO_PULL_MODE_UP) {
            fprintf(pull_file, "0");
        } else {
            fprintf(pull_file, "1");
        }
        fclose(pull_file);      
    }

    // IRQ edge detection
    char edge_path[50];
    snprintf(edge_path, sizeof(edge_path), "/sys/class/gpio/gpio%d/edge", pin);
    FILE *edge_file = fopen(edge_path, "w");
    if (edge_file == NULL) {
        printf("Failed to open GPIO edge file");
        exit(-1);
    }   
    switch (irq_mode) {
        case BSP_GPIO_IRQ_MODE_RISING:
            fprintf(edge_file, "rising");
            break;
        case BSP_GPIO_IRQ_MODE_FALLING:
            fprintf(edge_file, "falling");
            break;
        case BSP_GPIO_IRQ_MODE_RISING_FALLING:
            fprintf(edge_file, "both");
            break;
        default:
            break;         
    }
    fclose(edge_file);

    if( irq != NULL )
    {
        irq->pin = pin;
    }

    // const uint32_t modes[] = { GPIO_MODE_INPUT, GPIO_MODE_IT_RISING, GPIO_MODE_IT_FALLING,
    //                            GPIO_MODE_IT_RISING_FALLING };
    // const uint32_t pulls[] = { GPIO_NOPULL, GPIO_PULLUP, GPIO_PULLDOWN };

    // gpio_t gpio = {
    //     .pin = pin, .mode = modes[irq_mode], .pull = pulls[pull_mode], .speed = GPIO_SPEED_FREQ_LOW, .alternate = 0
    // };

    // if( irq != NULL )
    // {
    //     irq->pin = pin;
    // }

    // gpio_init( &gpio, GPIO_PIN_RESET, irq );
}

void hal_gpio_init_out( const hal_gpio_pin_names_t pin, const uint32_t value )
{
    if (pin == NC) {
        return;
    }
    // Export Pin
    FILE *export_file = fopen("/sys/class/gpio/export", "w");
    if (export_file == NULL) {
        printf("Failed to open GPIO export file");
        exit(-1);
    }
    fprintf(export_file, "%d", pin);
    fclose(export_file);

    // Set direction
    char direction_path[50];
    snprintf(direction_path, sizeof(direction_path), "/sys/class/gpio/gpio%d/direction", pin);
    FILE *direction_file = fopen(direction_path, "w");
    if (direction_file == NULL) {
        printf("Failed to open GPIO direction file");
        exit(-1);
    }
    fprintf(direction_file, "out");
    fclose(direction_file);
    // gpio_t gpio = {
    //     .pin = pin, .mode = GPIO_MODE_OUTPUT_PP, .pull = GPIO_NOPULL, .speed = GPIO_SPEED_FREQ_LOW, .alternate = 0
    // };
    // gpio_init( &gpio, ( value != 0 ) ? GPIO_PIN_SET : GPIO_PIN_RESET, NULL );
}

void hal_gpio_irq_attach( const hal_gpio_irq_t* irq )
{
    // if( ( irq != NULL ) && ( irq->callback != NULL ) )
    // {
    //     gpio_irq[( irq->pin ) & 0x0F] = irq;
    // }
    if( ( irq != NULL ) && ( irq->callback != NULL ) ) {
        pthread_create(&thread_id[irq->pin],NULL,irq_handler,irq);
    }
}

void hal_gpio_irq_deatach( const hal_gpio_irq_t* irq )
{
    // if( irq != NULL )
    // {
    //     gpio_irq[( irq->pin ) & 0x0F] = NULL;
    // }
    if( (irq != NULL) && (irq->pin < sizeof(thread_id)))
    {
        pthread_kill(thread_id[irq->pin],0);
    }
}

void hal_gpio_irq_enable( void )
{
    // HAL_NVIC_EnableIRQ( EXTI0_IRQn );
    // HAL_NVIC_EnableIRQ( EXTI1_IRQn );
    // HAL_NVIC_EnableIRQ( EXTI2_IRQn );
    // HAL_NVIC_EnableIRQ( EXTI3_IRQn );
    // HAL_NVIC_EnableIRQ( EXTI4_IRQn );
    // HAL_NVIC_EnableIRQ( EXTI9_5_IRQn );
    // HAL_NVIC_EnableIRQ( EXTI15_10_IRQn );
}

void hal_gpio_irq_disable( void )
{
//     // HAL_NVIC_DisableIRQ( EXTI0_IRQn );
//     // HAL_NVIC_DisableIRQ( EXTI1_IRQn );
//     // HAL_NVIC_DisableIRQ( EXTI2_IRQn );
//     // HAL_NVIC_DisableIRQ( EXTI3_IRQn );
//     // HAL_NVIC_DisableIRQ( EXTI4_IRQn );
//     // HAL_NVIC_DisableIRQ( EXTI9_5_IRQn );
//     // HAL_NVIC_DisableIRQ( EXTI15_10_IRQn );
}

// //
// // MCU pin state control
// //

void hal_gpio_set_value( const hal_gpio_pin_names_t pin, const uint32_t value )
{
    if (pin == NC) {
        return;
    }
    char value_path[50];
    char output_value[1];
    snprintf(value_path, sizeof(value_path), "/sys/class/gpio/gpio%d/value", pin);
    FILE *value_file = fopen(value_path, "w");
    if (value_file == NULL) {
        printf("Set Value: Failed to open GPIO value file %d\n",pin);
        exit(-1);
    }
    snprintf(output_value, sizeof(output_value), "%d", value);
    fprintf(value_file, output_value);
    fclose(value_file);
    // GPIO_TypeDef* gpio_port = ( GPIO_TypeDef* ) ( AHB2PERIPH_BASE + ( ( pin & 0xF0 ) << 6 ) );

    // HAL_GPIO_WritePin( gpio_port, ( 1 << ( pin & 0x0F ) ), ( value != 0 ) ? GPIO_PIN_SET : GPIO_PIN_RESET );
}

uint32_t hal_gpio_get_value( const hal_gpio_pin_names_t pin )
{
    char value_path[50];
    char readbuf[1];

    snprintf(value_path, sizeof(value_path), "/sys/class/gpio/gpio%d/value", pin);
    int value_file = open(value_path, O_WRONLY);
    if (value_file == -1) {
        printf("Failed to open GPIO value file\n");
        exit(-1);
    }    
    read(value_file, readbuf, sizeof(readbuf));
    // GPIO_TypeDef* gpio_port = ( GPIO_TypeDef* ) ( AHB2PERIPH_BASE + ( ( pin & 0xF0 ) << 6 ) );

    // return ( HAL_GPIO_ReadPin( gpio_port, ( ( 1 << ( pin & 0x0F ) ) ) ) != GPIO_PIN_RESET ) ? 1 : 0;
    return atoi(readbuf);
}

void hal_gpio_clear_pending_irq( const hal_gpio_pin_names_t pin )
{
//     switch( pin & 0x0F )
//     {
//     case 0:
//         NVIC_ClearPendingIRQ( EXTI0_IRQn );
//         break;
//     case 1:
//         NVIC_ClearPendingIRQ( EXTI1_IRQn );

//         break;
//     case 2:
//         NVIC_ClearPendingIRQ( EXTI2_IRQn );
//         break;
//     case 3:
//         NVIC_ClearPendingIRQ( EXTI3_IRQn );
//         break;
//     case 4:
//         NVIC_ClearPendingIRQ( EXTI4_IRQn );
//         break;
//     case 5:
//     case 6:
//     case 7:
//     case 8:
//     case 9:
//         NVIC_ClearPendingIRQ( EXTI9_5_IRQn );
//         break;
//     default:
//         NVIC_ClearPendingIRQ( EXTI15_10_IRQn );
//         break;
//     }
// NO IRQ SUPPORT
}

void hal_gpio_enable_clock( const hal_gpio_pin_names_t pin )
{
    // GPIO_TypeDef* gpio_port = ( GPIO_TypeDef* ) ( AHB2PERIPH_BASE + ( ( pin & 0xF0 ) << 6 ) );

    // if( gpio_port == GPIOA )
    // {
    //     __HAL_RCC_GPIOA_CLK_ENABLE( );
    // }
    // else if( gpio_port == GPIOB )
    // {
    //     __HAL_RCC_GPIOB_CLK_ENABLE( );
    // }
    // else if( gpio_port == GPIOC )
    // {
    //     __HAL_RCC_GPIOC_CLK_ENABLE( );
    // }
    // else if( gpio_port == GPIOD )
    // {
    //     __HAL_RCC_GPIOD_CLK_ENABLE( );
    // }
    // else if( gpio_port == GPIOE )
    // {
    //     __HAL_RCC_GPIOE_CLK_ENABLE( );
    // }
    // else if( gpio_port == GPIOH )
    // {
    //     __HAL_RCC_GPIOH_CLK_ENABLE( );
    // }
    // NO CLOCK SUPPORT
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

// static void gpio_init( const gpio_t* gpio, const uint32_t value, const hal_gpio_irq_t* irq )
// {
//     GPIO_InitTypeDef gpio_local;
//     GPIO_TypeDef*    gpio_port = ( GPIO_TypeDef* ) ( AHB2PERIPH_BASE + ( ( gpio->pin & 0xF0 ) << 6 ) );

//     gpio_local.Pin       = ( 1 << ( gpio->pin & 0x0F ) );
//     gpio_local.Mode      = gpio->mode;
//     gpio_local.Pull      = gpio->pull;
//     gpio_local.Speed     = gpio->speed;
//     gpio_local.Alternate = gpio->alternate;

//     if( gpio_port == GPIOA )
//     {
//         __HAL_RCC_GPIOA_CLK_ENABLE( );
//     }
//     else if( gpio_port == GPIOB )
//     {
//         __HAL_RCC_GPIOB_CLK_ENABLE( );
//     }
//     else if( gpio_port == GPIOC )
//     {
//         __HAL_RCC_GPIOC_CLK_ENABLE( );
//     }
//     else if( gpio_port == GPIOD )
//     {
//         __HAL_RCC_GPIOD_CLK_ENABLE( );
//     }
//     else if( gpio_port == GPIOE )
//     {
//         __HAL_RCC_GPIOE_CLK_ENABLE( );
//     }
//     else if( gpio_port == GPIOH )
//     {
//         __HAL_RCC_GPIOH_CLK_ENABLE( );
//     }

//     HAL_GPIO_WritePin( gpio_port, gpio_local.Pin, ( GPIO_PinState ) value );
//     HAL_GPIO_Init( gpio_port, &gpio_local );

//     if( ( gpio->mode == GPIO_MODE_IT_RISING ) || ( gpio->mode == GPIO_MODE_IT_FALLING ) ||
//         ( gpio->mode == GPIO_MODE_IT_RISING_FALLING ) )
//     {
//         hal_gpio_irq_attach( irq );
//         switch( gpio->pin & 0x0F )
//         {
//         case 0:
//             HAL_NVIC_SetPriority( EXTI0_IRQn, 0, 0 );
//             HAL_NVIC_EnableIRQ( EXTI0_IRQn );
//             break;
//         case 1:
//             HAL_NVIC_SetPriority( EXTI1_IRQn, 0, 0 );
//             HAL_NVIC_EnableIRQ( EXTI1_IRQn );
//             break;
//         case 2:
//             HAL_NVIC_SetPriority( EXTI2_IRQn, 0, 0 );
//             HAL_NVIC_EnableIRQ( EXTI2_IRQn );
//             break;
//         case 3:
//             HAL_NVIC_SetPriority( EXTI3_IRQn, 0, 0 );
//             HAL_NVIC_EnableIRQ( EXTI3_IRQn );
//             break;
//         case 4:
//             HAL_NVIC_SetPriority( EXTI4_IRQn, 0, 0 );
//             HAL_NVIC_EnableIRQ( EXTI4_IRQn );
//             break;
//         case 5:
//         case 6:
//         case 7:
//         case 8:
//         case 9:
//             HAL_NVIC_SetPriority( EXTI9_5_IRQn, 0, 0 );
//             HAL_NVIC_EnableIRQ( EXTI9_5_IRQn );
//             break;
//         default:
//             HAL_NVIC_SetPriority( EXTI15_10_IRQn, 0, 0 );
//             HAL_NVIC_EnableIRQ( EXTI15_10_IRQn );
//             break;
//         }
//     }
// }
// IRQ SUPPORT DISABLED
/******************************************************************************/
/* STM32L4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l4xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles EXTI line0 interrupt.
 */
// void EXTI0_IRQHandler( void )
// {
//     /* USER CODE BEGIN EXTI0_IRQn 0 */

//     /* USER CODE END EXTI0_IRQn 0 */
//     HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_0 );
//     /* USER CODE BEGIN EXTI0_IRQn 1 */

//     /* USER CODE END EXTI0_IRQn 1 */
// }

// /**
//  * @brief This function handles EXTI line1 interrupt.
//  */
// void EXTI1_IRQHandler( void )
// {
//     /* USER CODE BEGIN EXTI1_IRQn 0 */

//     /* USER CODE END EXTI1_IRQn 0 */
//     HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_1 );

//     /* USER CODE BEGIN EXTI1_IRQn 1 */

//     /* USER CODE END EXTI1_IRQn 1 */
// }

// /**
//  * @brief This function handles EXTI line2 interrupt.
//  */
// void EXTI2_IRQHandler( void )
// {
//     /* USER CODE BEGIN EXTI2_IRQn 0 */

//     /* USER CODE END EXTI2_IRQn 0 */
//     HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_2 );
//     /* USER CODE BEGIN EXTI2_IRQn 1 */

//     /* USER CODE END EXTI2_IRQn 1 */
// }

// /**
//  * @brief This function handles EXTI line3 interrupt.
//  */
// void EXTI3_IRQHandler( void )
// {
//     /* USER CODE BEGIN EXTI3_IRQn 0 */

//     /* USER CODE END EXTI3_IRQn 0 */
//     HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_3 );
//     /* USER CODE BEGIN EXTI3_IRQn 1 */

//     /* USER CODE END EXTI3_IRQn 1 */
// }

// /**
//  * @brief This function handles EXTI line4 interrupt.
//  */
// void EXTI4_IRQHandler( void )
// {
//     /* USER CODE BEGIN EXTI4_IRQn 0 */

//     /* USER CODE END EXTI4_IRQn 0 */
//     HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_4 );
//     /* USER CODE BEGIN EXTI4_IRQn 1 */

//     /* USER CODE END EXTI4_IRQn 1 */
// }

// /**
//  * @brief This function handles EXTI line[9:5] interrupts.
//  */
// void EXTI9_5_IRQHandler( void )
// {
//     /* USER CODE BEGIN EXTI9_5_IRQn 0 */

//     /* USER CODE END EXTI9_5_IRQn 0 */
//     HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_5 );
//     HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_6 );
//     HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_7 );
//     HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_8 );
//     HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_9 );
//     /* USER CODE BEGIN EXTI9_5_IRQn 1 */

//     /* USER CODE END EXTI9_5_IRQn 1 */
// }
// /**
//  * @brief This function handles EXTI line[15:10] interrupts.
//  */
// void EXTI15_10_IRQHandler( void )
// {
//     /* USER CODE BEGIN EXTI15_15_IRQn 0 */

//     /* USER CODE END EXTI15_15_IRQn 0 */
//     HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_10 );
//     HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_11 );
//     HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_12 );
//     HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_13 );
//     HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_14 );
//     HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_15 );
//     /* USER CODE BEGIN EXTI15_15_IRQn 1 */

//     /* USER CODE END EXTI15_15_IRQn 1 */
// }

// void HAL_GPIO_EXTI_Callback( uint16_t gpio_pin )
// {
//     uint8_t callback_index = 0;

//     if( gpio_pin > 0 )
//     {
//         while( gpio_pin != 0x01 )
//         {
//             gpio_pin = gpio_pin >> 1;
//             callback_index++;
//         }
//     }

//     if( ( gpio_irq[callback_index] != NULL ) && ( gpio_irq[callback_index]->callback != NULL ) )
//     {
//         gpio_irq[callback_index]->callback( gpio_irq[callback_index]->context );
//     }
// }
//
/* --- EOF ------------------------------------------------------------------ */
