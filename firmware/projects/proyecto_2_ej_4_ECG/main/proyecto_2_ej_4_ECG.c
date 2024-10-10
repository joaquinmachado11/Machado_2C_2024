/*! @mainpage Proyecto 2 ejercicio 4
 *
 * @section genDesc General description
 * Convierta una señal digital de un ECG (provista por la cátedra) en una señal analógica 
 * y visualice esta señal utilizando el osciloscopio que acaba de implementar. Se sugiere 
 * utilizar el potenciómetro para conectar la salida del DAC a la entrada CH1 del AD.
 *
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *>
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
#define BUFFER_SIZE 231
#define FREC_DE_MUESTREO_DAC 20000 // 500 Hz en useg
#define FREC_DE_MUESTREO_PLOTTER 10000 // 250 Hz en useg
uint16_t voltaje; // en mV
/*==================[internal data definition]===============================*/
uint8_t i = 0;
TaskHandle_t ADC_task_handle = NULL;
TaskHandle_t DAC_task_handle = NULL;

const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};
/*==================[internal functions declaration]=========================*/
void funcTimerADC(){
    vTaskNotifyGiveFromISR(ADC_task_handle, pdFALSE);
}

void funcTimerDAC(){
    vTaskNotifyGiveFromISR(DAC_task_handle, pdFALSE);
}

static void DAC_convert(void *pvParameter){ // conversion DAC
	while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogOutputWrite((uint8_t) ecg[i]);
	    i = i + 1;

	    if (i == BUFFER_SIZE)
		    i = 0;
    }
}

static void ADC_convert (void *pvParameter){ // conversion ADC
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		AnalogInputReadSingle(CH1, &voltaje);

		// Muestro los datos por serial monitor con el formato que interpreta el ploter descargado desde VS Code
		
        UartSendString(UART_PC, ">ECG: ");
		UartSendString(UART_PC, (char*)UartItoa(voltaje, 10));
		UartSendString(UART_PC, "\r\n");
        
       //printf(">ECG: %d, da: %d\r\n", voltaje, ecg[i]);
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

	// Inicializacion DAC
	AnalogOutputInit();

	// Configuracion de timer
	timer_config_t timer_ADC = {
        .timer = TIMER_A,
        .period = FREC_DE_MUESTREO_PLOTTER,
        .func_p = funcTimerADC,
        .param_p = NULL
    };
    TimerInit(&timer_ADC);

    timer_config_t timer_DAC = {
        .timer = TIMER_B,
        .period = FREC_DE_MUESTREO_DAC,
        .func_p = funcTimerDAC,
        .param_p = NULL
    };
    TimerInit(&timer_DAC);

	// Inicializacion de UART
    serial_config_t uart = {
        .port = UART_PC,
        .baud_rate = 115200,
        .func_p = NULL,
        .param_p = NULL
    };
    UartInit(&uart);

	// Creacion de tareas
	xTaskCreate(&ADC_convert, "Conversion ADC", 4096, NULL, 5, &ADC_task_handle);
    xTaskCreate(&DAC_convert, "Conversion DAC", 2048, NULL, 5, &DAC_task_handle);

	// Inicializacion de timer
	TimerStart(timer_ADC.timer);
    TimerStart(timer_DAC.timer);
}
/*==================[end of file]============================================*/