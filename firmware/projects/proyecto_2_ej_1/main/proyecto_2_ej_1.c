/*! @mainpage Proyecto 2 ejercicio 1
 *
 * @section genDesc General Description
 *
 * 1) Muestra la distancia medida con un sensor de ultrasonido utilizando los leds de la siguiente manera:
 * 	- Si la distancia es menor a 10 cm, apagar todos los LEDs.
 * 	- Si la distancia está entre 10 y 20 cm, encender el LED_1.
 * 	- Si la distancia está entre 20 y 30 cm, encender el LED_2 y LED_1.
 * 	- Si la distancia es mayor a 30 cm, encender el LED_3, LED_2 y LED_1.
 * 2) Muestra el valor de distancia en cm utilizando el display LCD.
 * 3) TEC1 activa y detiene la medición.
 * 4) TEC2 mantiene el resultado (“HOLD”).
 * 5) Refresco de medición: 1 s
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_LED_1 300
#define CONFIG_BLINK_PERIOD_LED_2 600
#define CONFIG_BLINK_PERIOD_LED_3 1000
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
    xTaskCreate(&Led1Task, "LED_1", 512, NULL, 5, NULL); // (funcion, espacio asignado, NULL, prioridad, NULL)
    xTaskCreate(&Led2Task, "LED_2", 512, NULL, 5, NULL);
    xTaskCreate(&Led3Task, "LED_3", 512, NULL, 5, NULL);
}
/*==================[end of file]============================================*/