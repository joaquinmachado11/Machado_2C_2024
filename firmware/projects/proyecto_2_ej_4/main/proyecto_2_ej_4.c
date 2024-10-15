/*! @mainpage Proyecto 2 ejercicio 4
 *
 * @section genDesc General description
 * Diseñar e implementar una aplicación, basada en el driver analog_io_mcu.h 
 * y el driver de transmisión serie uart_mcu.h, que digitalice una señal analógica y la transmita 
 * a un graficador de puerto serie de la PC. Se debe tomar la entrada CH1 del conversor AD y la 
 * transmisión se debe realizar por la UART conectada al puerto serie de la PC, en un formato 
 * compatible con un graficador por puerto serie. 
 *
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Joaquin Machado (joaquin.machado@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <analog_io_mcu.h> 
#include <uart_mcu.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
/*==================[macros and definitions]=================================*/
/** @def FREC_DE_MUESTREO
 *  @brief Frecuencia de muestreo, expresada en microsegundos.
 *  @details Equivale a 500 Hz.
 */
#define FREC_DE_MUESTREO 20000

/** @var voltaje
 *  @brief Valor del voltaje leído del canal analógico en milivoltios.
 */
uint16_t voltaje;

/*==================[internal data definition]===============================*/

/** @var ADC_task_handle
 *  @brief Manejador de la tarea de conversión ADC.
 */
TaskHandle_t ADC_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

/**
 * @brief Notifica a la tarea ADC desde la interrupción del temporizador.
 * @details Esta función se llama desde una rutina de interrupción y
 *          notifica a la tarea ADC para que procese la siguiente lectura.
 */
void funcTimerADC(){
    vTaskNotifyGiveFromISR(ADC_task_handle, pdFALSE);
}

/**
 * @brief Función de conversión ADC.
 * @param pvParameter Parámetro pasado a la tarea (no utilizado).
 * @details Lee un valor analógico del canal 1 y lo envía al monitor serial en formato legible 
 *          por un ploter descargado desde VS Code.
 */
static void ADC_convert(void *pvParameter){ 
    while(1){
        // Espera a ser notificada por la interrupción del temporizador
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        // Lee el valor del canal analógico y lo almacena en la variable voltaje
        AnalogInputReadSingle(CH1, &voltaje);

        // Envío de los datos al monitor serial
        UartSendString(UART_PC, ">Analog In: ");
        UartSendString(UART_PC, (char*)UartItoa(voltaje, 10));
        UartSendString(UART_PC, "\r\n");
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
	// Configuracion de ADC
	analog_input_config_t analogIn = {
		.input = CH1,
		.mode = ADC_SINGLE
	};

	// Inicializacion ADC
	AnalogInputInit(&analogIn);

	// Configuracion de timer
	timer_config_t timer_ADC = {
        .timer = TIMER_A,
        .period = FREC_DE_MUESTREO,
        .func_p = funcTimerADC,
        .param_p = NULL
    };
    TimerInit(&timer_ADC);

	// Inicializacion de UART
    serial_config_t uart = {
        .port = UART_PC,
        .baud_rate = 9600,
        .func_p = NULL,
        .param_p = NULL
    };
    UartInit(&uart);

	// Creacion de tareas
	xTaskCreate(&ADC_convert, "Conversion ADC", 2048, NULL, 5, &ADC_task_handle);

	// Inicializacion de timer
	TimerStart(timer_ADC.timer);
}
/*==================[end of file]============================================*/