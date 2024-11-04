/*! @mainpage Examen parcial
 *
 * @section genDesc General Description
 *
 * Se pretende diseñar un dispositivo basado en la ESP-EDU que permita detectar 
 * eventos peligrosos para ciclistas.  El sistema está compuesto por un acelerómetro analógico montado sobre el casco y un
 * HC-SR04 ubicado en la parte trasera de la bicicleta.
 *  Se indicará mediante los leds de la placa la distancia de los vehículos a la bicicleta.
 * Además activa una alarma sonora mediante un buzzer activo en caso de precaucion y peligro.
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
 * | 04/11/2024 | Document creation		                         |
 *
 * @author Joaquin Machado (joaquin.machado@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "buzzer.h"
#include <analog_io_mcu.h> 
/*==================[macros and definitions]=================================*/
#define TIEMPO_MUESTREO_DISTANCIA 500 // en ms
#define TIEMPO_MUESTREO_ACELEROMETRO 10 //en ms
#define GPIO_X GPIO_1
#define GPIO_Y GPIO_2
#define GPIO_Z GPIO_3
/*==================[internal data definition]===============================*/
TaskHandle_t distancia_task_handle = NULL;
TaskHandle_t aceleracion_task_handle = NULL;
uint8_t tension_X;	// Tensiones que devuelve el acelerometro
uint8_t tension_Y;
uint8_t tension_Z;
uint8_t tension_XYZ;
uint8_t distancia;
uint8_t sensibilidad = 0.3; // V/G
bool hayCaida = false;
/*==================[internal functions declaration]=========================*/
void funcTimerDistancia(){
    vTaskNotifyGiveFromISR(distancia_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada a pantalla */
}

void funcTimerAceleracion(){
    vTaskNotifyGiveFromISR(aceleracion_task_handle, pdFALSE);
}

static void enviarAdvertencia(){
	if (distancia < 500 && distancia > 300)
		UartSendString(UART_CONNECTOR, "Precaución, vehículo cerca.");

	if (distancia < 300)
		UartSendString(UART_CONNECTOR, "Peligro, vehículo cerca.");

	if (hayCaida){
		hayCaida = false;
		UartSendString(UART_CONNECTOR, "Caída detectada.");
	}
}

static void medirDistancia (void *pvParameter){
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
		distancia = HcSr04ReadDistanceInCentimeters();

		if (distancia > 500) {
            LedOn(LED_1);
            LedOff(LED_2);
            LedOff(LED_3);
            } else 
		if (distancia < 500 && distancia > 300) {
            LedOn(LED_1);
            LedOn(LED_2);
            LedOff(LED_3);
			enviarAdvertencia();
            } else 
		if (distancia < 300){
            LedOn(LED_1);
            LedOn(LED_2);
            LedOn(LED_3);
			enviarAdvertencia();
            }
        }
	}

uint8_t tension2AcelerationConversion(){
	return tension_XYZ/sensibilidad;
}

static void obtenerAceleracion (void *pvParameter){		// a paritr
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
		
		tension_XYZ = tension_X + tension_Y + tension_Z;
		if (tension2AcelerationConversion() >= 4){
			hayCaida = true;
			enviarAdvertencia();
		}
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	// Inicializacion de periféricos
    HcSr04Init(GPIO_3, GPIO_2);
    LedsInit();
	BuzzerInit(GPIO_20);
	GPIOInit(GPIO_X, GPIO_INPUT);
	GPIOInit(GPIO_Y, GPIO_INPUT);
	GPIOInit(GPIO_Z, GPIO_INPUT);

	// Configuracion de ADC
	analog_input_config_t analogIn = {
		.input = CH1,
		.mode = ADC_SINGLE
	};

	// Inicializacion ADC
	AnalogInputInit(&analogIn);

    // Inicializacion de UART
    serial_config_t uart = {
        .port = UART_CONNECTOR,
        .baud_rate = 9600,	// ver el del modulo BT
        .func_p = NULL,
        .param_p = NULL
    };
    UartInit(&uart);

    // Inicialización de timers 
    timer_config_t timer_HCSR04 = {
        .timer = TIMER_A,
        .period = TIEMPO_MUESTREO_DISTANCIA,
        .func_p = funcTimerDistancia,
        .param_p = NULL
    };
    TimerInit(&timer_HCSR04);

    timer_config_t timer_acelerometro = {
        .timer = TIMER_B,
        .period = TIEMPO_MUESTREO_ACELEROMETRO,
        .func_p = funcTimerAceleracion,
        .param_p = NULL
    };
    TimerInit(&timer_acelerometro);

    // Creacion de tareas
    xTaskCreate(&medirDistancia, "Medicion de distancia", 2048, NULL, 5, &distancia_task_handle);
    xTaskCreate(&obtenerAceleracion, "Obtencio de aceleracion", 2048, NULL, 5, &aceleracion_task_handle);

	// Inicio de los timers
    TimerStart(timer_acelerometro.timer);
    TimerStart(timer_HCSR04.timer);
}
/*==================[end of file]============================================*/