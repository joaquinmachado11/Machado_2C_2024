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
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct 
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal functions declaration]=========================*/
/** @fn void GPIO_config (gpioConf_t *gpio)
 * @brief Inicializa los pines GPIO
 * @param[in] gpio: (poner que es)
 * @return void
 */
void GPIO_config (gpioConf_t *gpio){
	for (uint8_t i=0; i<sizeof(gpio); i++)
	{
		GPIOInit(gpio[i].pin, gpio[i].dir);
	}
}

/** @fn int8_t  convertToBcdArray (uint32_t, uint8_t, uint8_t *)
 * @brief Convierte un numero a un arreglo de BCD
 * @param[in]
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

/** @fn void BCDtoGPIO(uint8_t, gpioConf_t *)
 * @brief Convierte un array de BCD a salidas GPIO
 * @param
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
 * @param
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

/*
	Escriba una función que reciba un dato de 32 bits,  la cantidad de dígitos de salida 
y dos vectores de estructuras del tipo  gpioConf_t. Uno de estos vectores es igual 
al definido en el punto anterior y el otro vector mapea los puertos con el dígito del 
LCD a donde mostrar un dato:
	- Dígito 1 -> GPIO_19
	- Dígito 2 -> GPIO_18
	- Dígito 3 -> GPIO_9
*/