/*! @mainpage Proyecto 1 ejercicio 6
 *
 * @section genDesc programa que recibe un dato de 32 bits,  y lo muestra en unn display LCD
 *
 * @section hardConn Hardware Connection
 *
 * |    Periferico  |   EDU-ESP   	|
 * |:--------------:|:--------------|
 * | 	D1   	 	| 	GPIO_20		|
 * | 	D2   	 	| 	GPIO_21		|
 * | 	D3   	 	| 	GPIO_22		|
 * | 	D4   	 	| 	GPIO_23		|
 * | 	SEL_1    	| 	GPIO_19		|
 * | 	SEL_2  	 	| 	GPIO_18		|
 * | 	SEL_3  	 	| 	GPIO_9		|
 * | 	+5V  	 	| 	  +5V 		|
 * | 	GND  	 	| 	  GND 		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 30/09/2023 | Document creation		                         |
 *
 * @author Joaquin Machado (joaquin.machado@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
/** @struct gpioConf_t
 *  @brief Estructura para pines GPIO 
 */
typedef struct 
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal functions declaration]=========================*/
/** @fn void GPIO_config (gpioConf_t *gpio)
 * @brief Inicializa los pines GPIO
 * @param[in] gpio: puntero a struct de objetos gpio e io
 * @return void
 */
void GPIO_config (gpioConf_t *gpio){
	for (uint8_t i=0; i<sizeof(gpio); i++)
	{
		GPIOInit(gpio[i].pin, gpio[i].dir);
	}
}

/** @fn int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
 * @brief Convierte un numero a un arreglo de BCD
 * @param[in] data: dato 
 * @param[in] digits: cantidad de digitos del dato
 * @param[in] bcd_number puntero a vector de enteros de 8 bits donde se guardara cada digito del numero como bcd
 * @return int8_t
 */
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)		// Devuelve 1 o 0 si fue exitoso o no
{																					// Tratar de siempre devolver un valor
	for (int i=0; i<digits; i++)
	{
		bcd_number[i] = data%10;
		data = data/10;
		//printf("%d", bcd_number[i]);
	}
	return 1;
}

/** @fn void BCDtoGPIO(uint8_t bcd_number, gpioConf_t *gpio)
 * @brief Convierte un array de BCD a salidas GPIO
 * @param[in] bcd_number: numero bcd de 8 bits
 * @param[in] gpio: puntero a struct gpioConf_t
 * @return void
 */
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

/** @fn void NumToLCD(uint32_t numero, uint8_t digitos, gpioConf_t *pins_BCD, gpioConf_t *pins_MUX)
 * @brief Muestra un numero en un display LCD
 * @param[in] numero: numero
 * @param[in] digitos: cantidad de digitos del numero
 * @param[in] pins_BCD: puntero a struct
 * @param[in] pins_MUX: puntero a struct para multiplexacion
 * @return void
 */
void NumToLCD(uint32_t numero, uint8_t digitos, gpioConf_t *pins_BCD, gpioConf_t *pins_MUX){
	uint8_t vec[digitos];
	
	convertToBcdArray(numero, digitos, vec);

	GPIO_config(pins_MUX);

	for (uint8_t i = 0; i<digitos; i++){
		BCDtoGPIO(vec[i], pins_BCD);

		GPIOOn(pins_MUX[i].pin);
		GPIOOff(pins_MUX[i].pin);
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){
	gpioConf_t pins_BCD[4]= {{GPIO_20, GPIO_OUTPUT},{GPIO_21, GPIO_OUTPUT},{GPIO_22, GPIO_OUTPUT},{GPIO_23, GPIO_OUTPUT}};
	uint32_t numero = 543;
	uint8_t digitos = 3;
	gpioConf_t pins_MUX[3] = {{GPIO_19, GPIO_OUTPUT},{GPIO_18, GPIO_OUTPUT},{GPIO_9, GPIO_OUTPUT}};

	NumToLCD(numero, digitos, pins_BCD, pins_MUX);
}
/*==================[end of file]============================================*/
