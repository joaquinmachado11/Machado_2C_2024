/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
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
#define ON 1
#define OFF 2
#define TOGGLE 3
/*==================[internal data definition]===============================*/

struct leds
{
    uint8_t mode;		// ON, OFF, TOGGLE
	uint8_t n_led;		// indica el número de led a controlar
	uint8_t n_ciclos;	// indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;   // indica el tiempo de cada ciclo
} my_leds; 

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void manejoDeLeds(struct leds *ptrLed)
{
	switch(ptrLed->mode)
	{
		case ON:
			LedOn(ptrLed->n_led);
		break;


		case OFF:
			LedOff(ptrLed->n_led);
		break;
		
		
		case TOGGLE:
			for (int i=0; i<ptrLed->n_ciclos; i++)
			{
				LedToggle(ptrLed->n_led);

				for (int j=0; j<ptrLed->periodo/100; j++)
				{
					vTaskDelay(100 / portTICK_PERIOD_MS);
				}
			}
		break;
	}
}

void app_main(void){
	LedsInit();				// Inicializo los perifericos
	struct leds *ptrLed;	// Creo un puntero a un objeto struct leds
	ptrLed = &my_leds;		// asigno el puntero al objeto my_leds
	
	//printf("Ingrese el led a modificar:\r\n");	scanf("%c",&ptrLed->n_led);
	//printf("Ingrese el modo: ");	scanf("%c",&ptrLed->mode);
	
	ptrLed->n_led = 1;
	ptrLed->mode = 3;

	if (ptrLed->mode == 3)
	{
		ptrLed->n_ciclos = 60;
		ptrLed->periodo = 500;
	}

	manejoDeLeds(ptrLed);
}
/*==================[end of file]============================================*/

/*
Realice un función que reciba un puntero a una estructura LED
*/


//uint8_t aux;
//printf("Ingrese el led a modificar: ");	scanf("%i",&ptrLed->n_led);

//printf("Ingrese el modo: ");	scanf("%i",&ptrLed->mode);

//printf("Ingrese el periodo en ms: ");		scanf("%i",&ptrLed->periodo);

//printf("Ingrese la cantidad de ciclos: "); 	scanf("%i",&ptrLed->ciclos);