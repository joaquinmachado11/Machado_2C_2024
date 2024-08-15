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
Escriba una función que reciba un dato de 32 bits,  la cantidad de dígitos de salida 
y un puntero a un arreglo donde se almacene los n dígitos. La función deberá convertir 
el dato recibido a BCD, guardando cada uno de los dígitos de salida en el arreglo pasado 
como puntero.
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
	for (int i=0; i<digits; i++)
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