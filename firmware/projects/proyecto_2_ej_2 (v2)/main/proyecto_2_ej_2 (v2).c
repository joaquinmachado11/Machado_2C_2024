/*! @mainpage Proyecto 2 ejercicio 2
 *
 * @section genDesc General Description
 *
 * Nuevo proyecto en el que se modifiqua la actividad del punto 1 de manera de utilizar 
 * interrupciones para el control de las teclas y el control de tiempos (Timers).
 * Cambio con el otro ejercicio al agregar la tarea de manejo de teclas y maneja los leds desde 
 * la tarea de mostrar. 
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

/** @brief Tiempo de refresco de la pantalla LCD en milisegundos. */
#define TIEMPO_REFRESCO_PANTALLA 200

/** @brief Tiempo entre mediciones de distancia en milisegundos. */
#define TIEMPO_MEDICION 1000

/** @brief Constante para indicar el estado encendido. */
#define ON 1

/** @brief Constante para indicar el estado apagado. */
#define OFF 0

/** @brief Distancia medida por el sensor. */
uint16_t distancia;

/** @brief Estado del switch 1 (indica si la pantalla está encendida o apagada). */
bool estadoS1 = false;

/** @brief Estado del switch 2 (indica si se congela o no la medición de distancia). */
bool estadoS2 = false;

/** @brief Handle de la tarea encargada de mostrar la distancia en la pantalla. */
TaskHandle_t mostrar_task_handle = NULL;

/** @brief Handle de la tarea encargada de medir la distancia. */
TaskHandle_t medicion_task_handle = NULL;

/** @brief Handle de la tarea encargada de manejar las teclas. */
TaskHandle_t teclas_task_handle = NULL;

/*==================[internal functions declaration]=========================*/

/**
 * @brief Función que notifica a la tarea encargada de manejar la pantalla.
 * Se llama desde una interrupción del timer.
 */
void funcTimerPantalla() {
    vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);
}

/**
 * @brief Función que notifica a la tarea encargada de medir la distancia.
 * Se llama desde una interrupción del timer.
 */
void funcTimerMedir() {
    vTaskNotifyGiveFromISR(medicion_task_handle, pdFALSE);
}

/**
 * @brief Función que notifica a la tarea encargada de manejar las teclas.
 * Se llama desde una interrupción del timer.
 */
void funcTimerLEDs() {
    vTaskNotifyGiveFromISR(teclas_task_handle, pdFALSE);
}

/**
 * @brief Tarea que mide la distancia usando el sensor ultrasónico HC-SR04.
 * La tarea espera una notificación antes de realizar la medición.
 * @param pvParameter Parámetro de entrada (no utilizado).
 */
static void medirDistancia(void *pvParameter) {
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (estadoS2 == ON) {
            // No se realiza la medición cuando estadoS2 está en ON
        } else {
            distancia = HcSr04ReadDistanceInCentimeters();
        }
    }
}

/**
 * @brief Tarea que muestra la distancia en la pantalla LCD y controla los LEDs.
 * La tarea espera una notificación antes de actualizar la pantalla y los LEDs.
 * @param pvParameter Parámetro de entrada (no utilizado).
 */
static void mostrarDistancia(void *pvParameter) {
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

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

/**
 * @brief Tarea que maneja los estados de los switches.
 * La tarea espera una notificación antes de leer el estado de los switches.
 * @param pvParameter Parámetro de entrada (no utilizado).
 */
static void manejoDeTeclas(void *pvParameter) {
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (SwitchesRead() == SWITCH_1) {
            estadoS1 = !estadoS1;
        }

        if (SwitchesRead() == SWITCH_2) {
            estadoS2 = !estadoS2;
        }
    }
}

/*==================[external functions definition]==========================*/

/**
 * @brief Función principal del programa.
 * Inicializa los periféricos, timers y tareas del sistema.
 */
void app_main(void) {
    // Inicialización de periféricos
    SwitchesInit();
    HcSr04Init(GPIO_3, GPIO_2);
    LedsInit();
    LcdItsE0803Init();

    // Configuración de los timers
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

    // Creación de tareas
    xTaskCreate(&medirDistancia, "Medicion", 2048, NULL, 5, &medicion_task_handle);
    xTaskCreate(&mostrarDistancia, "Mostrar", 512, NULL, 5, &mostrar_task_handle);
    xTaskCreate(&manejoDeTeclas, "Teclas", 512, NULL, 5, &teclas_task_handle);

    // Inicio de los timers
    TimerStart(timer_pantalla.timer);
    TimerStart(timer_medicion.timer);
}
