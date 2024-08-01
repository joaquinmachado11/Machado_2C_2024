/*! @mainpage Blinking switch
 *
 * \section genDesc General Description
 *
 * This example makes LED_1 and LED_2 blink if SWITCH_1 or SWITCH_2 are pressed.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 500
/*==================[internal data definition]===============================*/
struct Tiempo 
	{
		uint8_t led1;
		uint8_t led2;
		uint8_t led3;
	} tiempo;
/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	uint8_t teclas;		// Variable entera de 8 bits que guarda el estado de las teclas.
	LedsInit();			// Inicializo LEDs de la placa.
	SwitchesInit();		// Inicializo switches de la placa.

	tiempo.led1 = 100;	// a partir de struct defino los tiempos para cada LED
	tiempo.led2 = 300;
	tiempo.led3 = 1000;
	
    while(1)    {
    	teclas  = SwitchesRead();		// Leo el estado de los switches y lo guardo en teclas
    	switch(teclas)					// switch con teclas
		{
    		case SWITCH_1:
				printf("LED 1 titila\n");
				while (teclas == SWITCH_1)	// mientras este activo SWITCH_1...
				{
    				LedToggle(LED_1);		// cambio el estado del LED
					vTaskDelay(tiempo.led1 / portTICK_PERIOD_MS);
					teclas  = SwitchesRead();	// Lee nuevamente el estado de los switches
				}
				LedOff(LED_1);		// Apago el LED la soltar el switch
    		break;
    		case SWITCH_2:
    			printf("LED 2 titila\n");
				while (teclas == SWITCH_2)
				{
    				LedToggle(LED_2);
					vTaskDelay(tiempo.led2 / portTICK_PERIOD_MS);
					teclas  = SwitchesRead();
				}
				LedOff(LED_2);
    		break;
			case SWITCH_1 | SWITCH_2:
				printf("LED 3 titila\n");
				while (teclas == (SWITCH_2 | SWITCH_1))
				{
    				LedToggle(LED_3);
					vTaskDelay(tiempo.led3 / portTICK_PERIOD_MS);
					teclas  = SwitchesRead();
				}
				LedOff(LED_3);
				break;
    	}
	}
}
