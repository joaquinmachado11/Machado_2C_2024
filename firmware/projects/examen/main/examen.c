/*! @mainpage Examen parcial
 *
 * @section genDesc General Description
 *
 * Se pretende diseñar un dispositivo basado en la ESP-EDU que permita detectar eventos peligrosos para ciclistas.  
 * El sistema está compuesto por un acelerómetro analógico montado sobre el casco y un HC-SR04 ubicado en la parte trasera de la bicicleta.
 * Se indicará mediante los leds de la placa la distancia de los vehículos a la bicicleta. Además activa una alarma sonora mediante un buzzer activo en caso de precaucion y peligro.
 * A partir de un modulo BT  se envian datos de advertencia de proximidad y caida.
 *
 * @section hardConn Hardware Connection
 *
 * |    BUZZER	    |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	VCC	 		| 	GPIO_20		|
 * | 	GND	 		| 	 GND		|
 * 
 * 
 * |    HC-sr40	    |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	  GPIO_2    |     TRIGGER	|
 * | 	  GPIO_3    |      ECHO		|
 * | 	  +5V       |      +5V		|
 * | 	  GND       |      GND	    |
 * 
 * 
 * |    HC-05	    |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	VCC	 		| 	GPIO_20		|
 * | 	GND	 		| 	 GND		|
 * | 	USB	 		|  USB to UART	|
 *
 * 
 * |  Acelerometro	|   ESP32   	|
 * |:--------------:|:--------------|
 * | 	  out_x     |      CH1	    |
 * | 	  out_y     |      CH2	    |
 * | 	  out_z     |      CH3	    |
 * | 	  VCC       |      VCC		|
 * | 	  GND       |      GND	    |
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
/** @def TIEMPO_MUESTREO_DISTANCIA
 *  @brief Frecuencia de muestreo para el sensor ultrasonido, expresada en milisegundos.
 */
#define TIEMPO_MUESTREO_DISTANCIA 500
/** @def TIEMPO_MUESTREO_DISTANCIA
 *  @brief Frecuencia de muestreo para el acelerometro, expresada en milisegundos.
 */
#define TIEMPO_MUESTREO_ACELEROMETRO 10 
/** @brief Variable que almacena el GPIO al que se conecta el buzzer */
#define GPIO_BUZZER GPIO_20	
/** @brief Variable que almacena el pin al que se conecta la aceleracion en X del acelerometro */
#define CH_X CH1	
/** @brief Variable que almacena el pin al que se conecta la aceleracion en Y del acelerometro */
#define CH_Y CH2
/** @brief Variable que almacena el pin al que se conecta la aceleracion en Z del acelerometro */
#define CH_Z CH3
/*==================[internal data definition]===============================*/
/** @brief Handle de la tarea medir distancia */
TaskHandle_t distancia_task_handle = NULL;
/** @brief Handle de la tarea obtener aceleracion */
TaskHandle_t aceleracion_task_handle = NULL;
/** @brief Variable que almacena la aceleracion umbral a la que se considera una caida */
uint8_t aceleracionDeCaida = 4; // [G]
/** @brief Variable que almacena la tension devuelta por el acelerometro en X */
uint8_t tension_X;	
/** @brief Variable que almacena la tension devuelta por el acelerometro en Y */
uint8_t tension_Y;
/** @brief Variable que almacena la tension devuelta por el acelerometro en Z */
uint8_t tension_Z;
/** @brief Variable que almacena la suma de tensiones devueltas por el acelerometro */
uint8_t tension_XYZ;
/** @brief  Distancia medida de la bicicleta al auto*/	
uint8_t distance2car;	
/** @brief  Sensibilidad del acelerometro en [V/G]*/	
uint8_t sensibilidad = 0.3;
/** @brief variable que sabe si hay caida */	
bool hayCaida = false;
/** @brief  Variable que sabe si hay que tener precaucion */	
bool precaucion = false;
/** @brief  variable que sabe si hay peligro*/	
bool peligro = false;
/*==================[internal functions declaration]=========================*/
/**
 * @brief Notifica a la tarea medir distancia desde la interrupción del temporizador.
 * @details Esta función se llama desde una rutina de interrupción y
 *          notifica a la tarea medir distancia para que procese la siguiente lectura.
 */
void funcTimerDistancia(){
    vTaskNotifyGiveFromISR(distancia_task_handle, pdFALSE); 
}

/**
 * @brief Notifica a la tarea obtener aceleracion desde la interrupción del temporizador.
 * @details Esta función se llama desde una rutina de interrupción y
 *          notifica a la tarea obtener aceleracion para que procese la siguiente lectura.
 */
void funcTimerAceleracion(){
    vTaskNotifyGiveFromISR(aceleracion_task_handle, pdFALSE);
}

/** @brief  Funcion que envia advertencia a traves del modulo BT */	
static void enviarAdvertencia(){	
	if (precaucion){
		UartSendString(UART_CONNECTOR, "Precaución, vehículo cerca.");
		precaucion = false;
	}

	if (peligro){
		UartSendString(UART_CONNECTOR, "Peligro, vehículo cerca.");
		peligro = false;
	}

	if (hayCaida){
		hayCaida = false;
		UartSendString(UART_CONNECTOR, "Caída detectada.");
	}
}

/** @brief  Funcion que maneja el prendido y apagado de los buzzer */
static void prenderBuzzer(){
	
}

/**
 * @brief Tarea que mide la distancia usando el sensor ultrasónico HC-SR04 y maneja los estados del LEDs, buzzer y envios de advertencia.
 * La tarea espera una notificación antes de realizar la medición.
 * @param pvParameter Parámetro de entrada (no utilizado).
 */
static void medirDistancia (void *pvParameter){		
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    
		distance2car = HcSr04ReadDistanceInCentimeters();	// en cm

		if (distance2car > 500) {
            LedOn(LED_1);
            LedOff(LED_2);
            LedOff(LED_3);
            } else 
		if (distance2car < 500 && distance2car > 300) {
            precaucion = true;
			LedOn(LED_1);
            LedOn(LED_2);
            LedOff(LED_3);
			enviarAdvertencia();
			prenderBuzzer();
            } else 
		if (distance2car < 300){
			peligro = true;
            LedOn(LED_1);
            LedOn(LED_2);
            LedOn(LED_3);
			enviarAdvertencia();
			prenderBuzzer();
            }
        }
	}

/**
 * @brief Convierte la suma de tensiones obtenidas por el acelerometro en un dato de aceleracion
 */
uint8_t tension2AcelerationConversion(){	
	return tension_XYZ/sensibilidad;
}

/**
 * @brief Tarea que mide la tensiones del acelerometro y detecta si hubo caida.
 * @param pvParameter Parámetro de entrada (no utilizado).
 */
static void obtenerAceleracion (void *pvParameter){		
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   
		
		AnalogInputReadSingle(CH_X, &tension_X);
		AnalogInputReadSingle(CH_Y, &tension_Y);
		AnalogInputReadSingle(CH_Z, &tension_Z);

		tension_XYZ = tension_X + tension_Y + tension_Z;

		if (tension2AcelerationConversion() >= aceleracionDeCaida){
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
	GPIOInit(GPIO_BUZZER, GPIO_OUTPUT);

	// Configuracion de ADCs
	analog_input_config_t analogInX = {
		.input = CH_X,
		.mode = ADC_SINGLE
	};
	analog_input_config_t analogInY = {
		.input = CH_Y,
		.mode = ADC_SINGLE
	};
	analog_input_config_t analogInZ = {
		.input = CH_Z,
		.mode = ADC_SINGLE
	};

	// Inicializacion ADCs
	AnalogInputInit(&analogInX);
	AnalogInputInit(&analogInY);
	AnalogInputInit(&analogInZ);

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
    xTaskCreate(&obtenerAceleracion, "Obtencion de aceleracion", 2048, NULL, 5, &aceleracion_task_handle);

	// Inicio de los timers
    TimerStart(timer_acelerometro.timer);
    TimerStart(timer_HCSR04.timer);
}
/*==================[end of file]============================================*/