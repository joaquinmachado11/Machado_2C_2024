/*! @mainpage Proyecto 2 ejercicio 1
 *
 * @section genDesc General Description
 *
 * Nuevo proyecto en el que se modifiqua la actividad del punto 1 de manera de utilizar 
 * interrupciones para el control de las teclas y el control de tiempos (Timers). 
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
#include "timer_mcu.h"
/*==================[macros and definitions]=================================*/
/** @brief Constante para indicar el estado encendido. */
#define ON 1

/** @brief Constante para indicar el estado apagado. */
#define OFF 0

/** @brief Intervalo de tiempo para refrescar la pantalla (en us) */
#define TIEMPO_REFRESCO_PANTALLA 500000

/** @brief Intervalo de tiempo para realizar mediciones (en us) */
#define TIEMPO_MEDICION 1000000

/** @brief Variable que almacena la distancia medida */
uint16_t distancia;

/** @brief Estado del switch 1 */
bool estadoS1 = false;

/** @brief Estado del switch 2 */
bool estadoS2 = false;

/** @brief Handle para la tarea asociada a la pantalla */
TaskHandle_t pantalla_task_handle = NULL;

/** @brief Handle para la tarea asociada a la medición */
TaskHandle_t medicion_task_handle = NULL;

/*==================[internal data definition]===============================*/
/*==================[internal functions declaration]=========================*/

static void evento_switch1(){
    estadoS1 =! estadoS1;
}

static void evento_switch2(){
    estadoS2 =! estadoS2;
}

/**
 * @brief Función que notifica a la tarea de la pantalla.
 * 
 * Se llama desde una interrupción para notificar a la tarea que maneja la pantalla.
 */
void funcTimerPantalla(){
    vTaskNotifyGiveFromISR(pantalla_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada a pantalla */
}

/**
 * @brief Función que notifica a la tarea de medición.
 * 
 * Se llama desde una interrupción para notificar a la tarea que realiza la medición de distancia.
 */
void funcTimerMedir(){
    vTaskNotifyGiveFromISR(medicion_task_handle, pdFALSE);
}

/**
 * @brief Función para medir la distancia utilizando el sensor HcSr04.
 * 
 * Esta tarea espera una notificación antes de realizar una medición. Si se detecta un cambio en el switch 2, 
 * alterna su estado. Si el switch 2 está desactivado, mide la distancia utilizando el sensor HcSr04.
 * 
 * @param[in] pvParameter Parámetro no utilizado.
 */
static void medirDistancia (void *pvParameter){
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */

        if (estadoS2 == ON){}
        
        else
            distancia = HcSr04ReadDistanceInCentimeters(); 
    }
}

/**
 * @brief Función para mostrar la distancia medida en la pantalla LCD.
 * 
 * Esta tarea espera una notificación antes de ejecutar el código. Si se activa el switch 1, apaga la pantalla.
 * Si el switch está desactivado, muestra la distancia medida en la pantalla LCD.
 * 
 * @param[in] pvParameter Parámetro no utilizado.
 */
static void mostrarDistancia(void *pvParameter){
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */

        if (estadoS1 == OFF) {
            LcdItsE0803Write(distancia);

            if (distancia < 10) {
                LedsOffAll();
            } else if (distancia < 20) {
                LedOn(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
            } else if (distancia < 30) {
                LedOn(LED_1);
                LedOn(LED_2);
                LedOff(LED_3);
            } else {
                LedOn(LED_1);
                LedOn(LED_2);
                LedOn(LED_3);
            }
        } else {
            LcdItsE0803Off();
            LedsOffAll();
        }
    }
} 

/*==================[external functions definition]==========================*/

/**
 * @brief Función principal de la aplicación.
 * 
 * Inicializa los periféricos, configura los timers y crea las tareas para medir la distancia, mostrarla en la pantalla
 * y gestionar los LEDs en función de la distancia. 
 * 
 * También inicializa y arranca los timers que enviarán notificaciones a las tareas correspondientes.
 */
void app_main(void){
	// Inicializacion de periféricos
    SwitchesInit();
    HcSr04Init(GPIO_3, GPIO_2);
    LedsInit();
    LcdItsE0803Init();

    // Inicialización de timers 
    timer_config_t timer_pantalla = {
        .timer = TIMER_A,
        .period = TIEMPO_REFRESCO_PANTALLA,
        .func_p = funcTimerPantalla,
        .param_p = NULL
    };
    TimerInit(&timer_pantalla);

    timer_config_t timer_medicion = {
        .timer = TIMER_B,
        .period = TIEMPO_MEDICION,
        .func_p = funcTimerMedir,
        .param_p = NULL
    };
    TimerInit(&timer_medicion);


    // Creacion de tareas
    xTaskCreate(&medirDistancia, "Medicion", 2048, NULL, 5, &medicion_task_handle);
    xTaskCreate(&mostrarDistancia, "Mostrar", 512, NULL, 5, &pantalla_task_handle);
    

    // Inicio interrupcion de switches SwitchActivInt(switch_t sw, void *ptr_int_func, void *args)
    SwitchActivInt(SWITCH_1, *evento_switch1, NULL);
    SwitchActivInt(SWITCH_2, *evento_switch2, NULL);

    // Inicio de los timers
    TimerStart(timer_pantalla.timer);
    TimerStart(timer_medicion.timer);
}
/*==================[end of file]============================================*/
