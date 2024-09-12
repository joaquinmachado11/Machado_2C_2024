/*! @mainpage Proyecto 2 ejercicio 2
 *
 * @section genDesc General Description
 *
 * Nuevo proyecto en el que se modifiqua la actividad del punto 1 de manera de utilizar 
 * interrupciones para el control de las teclas y el control de tiempos (Timers). 
 *
 * @section hardConn Hardware Connection
 *
 * |    EDU-CIAA-NXP  |   PERIFERICO   	|
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
#include "timer_mcu.h"
/*==================[macros and definitions]=================================*/
#define TIEMPO_REFRESCO_PANTALLA 200
#define TIEMPO_MEDICION 1000
#define ON 1
#define OFF 0

uint16_t distancia;
bool estadoS1 = false;
bool estadoS2 = false;
TaskHandle_t mostrar_task_handle = NULL;
TaskHandle_t medicion_task_handle = NULL;
TaskHandle_t teclas_task_handle = NULL;

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
void funcTimerPantalla(){
    vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada a pantalla */
}

void funcTimerMedir(){
    vTaskNotifyGiveFromISR(medicion_task_handle, pdFALSE);
}

void funcTimerLEDs(){
    vTaskNotifyGiveFromISR(teclas_task_handle, pdFALSE);
}

static void medirDistancia (void *pvParameter){
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */

        if (estadoS2 == ON){}
        
        else
            distancia = HcSr04ReadDistanceInCentimeters(); 
    }
}

static void mostrarDistancia(void *pvParameter){
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */

        if (estadoS1 == OFF){
            LcdItsE0803Write(distancia);

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
            
        else {
            LcdItsE0803Off();
            LedsOffAll();
        }
    }
} 

static void manejoDeTeclas(void *pvParameter){
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */

    while(1){
        if (SwitchesRead() == SWITCH_1){
            estadoS1 = !estadoS1;
        }

        if (SwitchesRead() == SWITCH_2){
            estadoS2 = !estadoS2;
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
	// Inicializacion
    SwitchesInit();
    HcSr04Init(GPIO_3, GPIO_2);
    LedsInit();
    LcdItsE0803Init();

    // Inicialización de timers 
    timer_config_t timer_pantalla = {
        .timer = TIMER_A,
        .period = TIEMPO_REFRESCO_PANTALLA,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_pantalla);

    timer_config_t timer_medicion = {
        .timer = TIMER_B,
        .period = TIEMPO_MEDICION,
        .func_p = FuncTimerB,
        .param_p = NULL
    };
    TimerInit(&timer_medicion);

    timer_config_t timer_teclas = {
        .timer = TIMER_C,
        .period = 400,
        .func_p = FuncTimerC,
        .param_p = NULL
    };
    TimerInit(&timer_teclas);

    // Creacion de tareas
    xTaskCreate(&medirDistancia, "Medicion", 2048, NULL, 5, &medicion_task_handle);
    xTaskCreate(&mostrarDistancia, "Mostrar", 512, NULL, 5, &mostrar_task_handle);
    xTaskCreate(&manejoDeTeclas, "Teclas", 512, NULL, 5, &teclas_task_handle);

    TimerStart(timer_pantalla.timer);
    TimerStart(timer_medicion.timer);
}
/*==================[end of file]============================================*/