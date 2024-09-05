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
#define CONFIG_OFF_PERIOD_LEDs 500  

#define TIEMPO_REFRESCO_PANTALLA 200
#define TIEMPO_MEDICION 1000

uint16_t distancia;
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
static void medirDistancia (void *pvParameter){
    while(1){
        if (SwitchesRead() != SWITCH_2){
            distancia = HcSr04ReadDistanceInCentimeters();
        }

        vTaskDelay(TIEMPO_MEDICION / portTICK_PERIOD_MS);
    }
}

static void mostrarDistancia(void *pvParameter){
    while(1){
        if (SwitchesRead() != SWITCH_1)
             LcdItsE0803Write(distancia);

        else LcdItsE0803Off();

        vTaskDelay(TIEMPO_REFRESCO_PANTALLA / portTICK_PERIOD_MS);
    }
} 

static void manejoDeLEDs(void *pvParameter){
    while(1){
        if (SwitchesRead() != SWITCH_1){
            if (distancia<10){
                LedsOffAll();
            }

            if ((distancia<20) && (distancia>=10)){
                LedOn(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
            }

            if((distancia<30) && (distancia>=20)){
                LedOn(LED_1);

                LedOn(LED_2);
                LedOff(LED_3);
            }

            if(30<=distancia){
                LedOn(LED_1);

                LedOn(LED_2);

                LedOn(LED_3);

            }
        }
        else LedsOffAll();
    
    vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
	// Inicializacion
    SwitchesInit();
    HcSr04Init(GPIO_3, GPIO_2);
    LedsInit();
    LcdItsE0803Init();

    // Creacion de tareas
    xTaskCreate(&medirDistancia, "Medicion", 2048, NULL, 5, NULL);
    xTaskCreate(&mostrarDistancia, "Mostrar", 512, NULL, 5, NULL);
    xTaskCreate(&manejoDeLEDs, "LEDs", 512, NULL, 5, NULL);
}
/*==================[end of file]============================================*/