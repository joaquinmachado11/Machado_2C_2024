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
 * |    EDU-CIAA-NXP  |   PERIFERICO   	|
 * |:----------------:|:----------------|
 * | 	  GPIO_2      |     TRIGGER		|
 * | 	  GPIO_3      |      ECHO		|
 * | 	  +5V         |      +5V		|
 * | 	  GND         |      GND	    |
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

/** @brief Intervalo de tiempo para refrescar la pantalla (en ms) */
#define TIEMPO_REFRESCO_PANTALLA 200

/** @brief Intervalo de tiempo para realizar mediciones (en ms) */
#define TIEMPO_MEDICION 1000

/** @brief Variable que almacena la distancia medida */
uint16_t distancia;

/** @brief Estado del switch 1 */
bool estadoS1 = false;

/** @brief Estado del switch 2 */
bool estadoS2 = false;

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/**
 * @brief Función para medir la distancia utilizando el sensor HcSr04.
 * 
 * Esta tarea se ejecuta en un bucle infinito. Si se detecta el estado del switch 2,
 * alterna su estado. Si el switch 2 está desactivado, mide la distancia utilizando
 * el sensor HcSr04 y la almacena en la variable `distancia`.
 * 
 * @param[in] pvParameter Parámetro no utilizado.
 */
static void medirDistancia (void *pvParameter){
    while(1){
        if (SwitchesRead() == SWITCH_2){
            estadoS2 = !estadoS2;
        }

        if (estadoS2 == true){}
        
        else
            distancia = HcSr04ReadDistanceInCentimeters(); 

        vTaskDelay(TIEMPO_MEDICION / portTICK_PERIOD_MS);
    }
}

/**
 * @brief Función para mostrar la distancia medida en la pantalla LCD.
 * 
 * Esta tarea se ejecuta en un bucle infinito. Si se activa el switch 1, apaga la pantalla.
 * Si el switch está desactivado, muestra la distancia medida en la pantalla LCD.
 * 
 * @param[in] pvParameter Parámetro no utilizado.
 */
static void mostrarDistancia(void *pvParameter){
    while(1){
        if (SwitchesRead() == SWITCH_1){
            estadoS1 = !estadoS1;
        }
        
        if (estadoS1 == true)
            LcdItsE0803Off();
        
        else
            LcdItsE0803Write(distancia);

        vTaskDelay(TIEMPO_REFRESCO_PANTALLA / portTICK_PERIOD_MS);
    }
}

/**
 * @brief Función para gestionar el encendido y apagado de los LEDs en función de la distancia medida.
 * 
 * Si el switch 1 está desactivado, esta tarea enciende o apaga los LEDs dependiendo de la distancia medida.
 * - Si la distancia es menor a 10, se apagan todos los LEDs.
 * - Si la distancia está entre 10 y 20, se enciende solo el LED 1.
 * - Si la distancia está entre 20 y 30, se encienden los LEDs 1 y 2.
 * - Si la distancia es mayor o igual a 30, se encienden todos los LEDs (1, 2 y 3).
 * 
 * @param[in] pvParameter Parámetro no utilizado.
 */
static void manejoDeLEDs(void *pvParameter){
    while(1){
        if (estadoS1 == false){
            if (distancia < 10){
                LedsOffAll();
            }

            if ((distancia < 20) && (distancia >= 10)){
                LedOn(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
            }

            if((distancia < 30) && (distancia >= 20)){
                LedOn(LED_1);
                LedOn(LED_2);
                LedOff(LED_3);
            }

            if(30 <= distancia){
                LedOn(LED_1);
                LedOn(LED_2);
                LedOn(LED_3);
            }
        }
        else {
            LedsOffAll();
        }
    
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
}

/*==================[external functions definition]==========================*/

/**
 * @brief Función principal de la aplicación.
 * 
 * Inicializa los periféricos y crea las tareas para medir la distancia, mostrarla en la pantalla
 * y gestionar los LEDs en función de la distancia.
 */
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
