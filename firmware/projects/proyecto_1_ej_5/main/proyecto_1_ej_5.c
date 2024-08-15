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

/*
Escribir una función que reciba como parámetro un dígito BCD y un vector de 
estructuras del tipo gpioConf_t. Incluya el archivo de cabecera gpio_mcu.h
*/

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

typedef struct 
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal functions declaration]=========================*/

void GPIO_config (gpioConf_t *gpio){
	for (uint8_t i=0; i<sizeof(gpio); i++)
	{
		GPIOInit(gpio[i].pin, gpio[i].dir);
	}
}

void BCDtoGPIO(uint8_t bcd_number, gpioConf_t *gpio){
	GPIO_config(gpio);

	uint8_t mask = 1;
	for (int j=0; j<sizeof(gpio); j++)
	{
		if ((bcd_number & mask) != 0)
		{
			GPIOOn(gpio[j].pin);
		}
		else 
		{
			GPIOOff(gpio[j].pin);
		}

		mask = mask << 1;
	}
}


/*==================[external functions definition]==========================*/
void app_main(void){
	
	gpioConf_t *gpio[4] = {{GPIO_20, GPIO_OUTPUT},{GPIO_21, GPIO_OUTPUT},{GPIO_22, GPIO_OUTPUT},{GPIO_23, GPIO_OUTPUT}};

	uint8_t bcd_number = 3; 

	BCDtoGPIO(&bcd_number, gpio);
}
/*==================[end of file]============================================*/
