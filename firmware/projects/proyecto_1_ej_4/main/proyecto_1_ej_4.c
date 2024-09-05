/*! @mainpage Proyecto 1 ejercicio 6
 *
 * @section genDesc General Description
 * A partir del dato de un numero y la cantidad de sus digitos,
 * grafica el numero en un diplay LCD.	
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
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/

int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)		// Devuelve 1 o 0 si fue exitoso o no
{																					// Tratar de siempre devolver un valor
	for (int i=digits-1; i<=0; i--)
	{
		bcd_number[i] = data%10;
		data = data/10;
		printf("%d", bcd_number[i]);
	}
	return 1;
}

void app_main(void){
	uint32_t data = 123;	// Dato de 32 bits
	uint8_t digits = 3;	// Cantidad de digitos de salida
	uint8_t *bcd_number[digits];	// Arreglo que guarda cada digito del dato como bcd

	convertToBcdArray(data, digits, &bcd_number);
}
/*==================[end of file]============================================*/