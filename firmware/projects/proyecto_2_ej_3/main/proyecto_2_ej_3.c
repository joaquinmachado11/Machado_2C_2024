/*! @mainpage Template
 *
 * @section genDesc General Description
 * Nuevo proyecto en el que modifique la actividad del punto 2 agregando ahora el puerto serie. 
 * Con las teclas “O” y “H”, replicar la funcionalidad de las teclas 1 y 2 de los puntos anteriores.
 * Usar “I” para cambiar la unidad de trabajo de “cm” a “pulgadas”
 * Usar “M” para implementar la visualización del máximo
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	  GPIO_2    |     TRIGGER	|
 * | 	  GPIO_3    |      ECHO		|
 * | 	  +5V       |      +5V		|
 * | 	  GND       |      GND	    |
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 19/09/2024 | Document creation		                         |
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
#include "uart_mcu.h"
/**
 * @file    distance_measurement.c
 * @brief   Implementación de medición de distancia y control de pantalla con FreeRTOS.
 * @details Este archivo contiene las definiciones y funciones para medir la distancia con un
 *          sensor HcSr04 y mostrarla en una pantalla LCD, utilizando tareas de FreeRTOS.
 */

/*==================[macros and definitions]=================================*/

/** @def ON
 *  @brief Constante para indicar el estado encendido.
 */
#define ON 1

/** @def OFF
 *  @brief Constante para indicar el estado apagado.
 */
#define OFF 0

/** @def TIEMPO_REFRESCO_PANTALLA
 *  @brief Intervalo de tiempo para refrescar la pantalla, en microsegundos.
 */
#define TIEMPO_REFRESCO_PANTALLA 500000

/** @def TIEMPO_MEDICION
 *  @brief Intervalo de tiempo para realizar mediciones, en microsegundos.
 */
#define TIEMPO_MEDICION 1000000

/** @var distancia
 *  @brief Variable que almacena la distancia medida por el sensor, en centímetros.
 */
uint16_t distancia;

/** @var estadoS1
 *  @brief Estado del switch 1, utilizado para encender o apagar la pantalla.
 */
bool estadoS1 = false;

/** @var estadoS2
 *  @brief Estado del switch 2, utilizado para activar o desactivar la medición.
 */
bool estadoS2 = false;

/** @var pantalla_task_handle
 *  @brief Manejador de la tarea asociada a la pantalla.
 */
TaskHandle_t pantalla_task_handle = NULL;

/** @var medicion_task_handle
 *  @brief Manejador de la tarea asociada a la medición.
 */
TaskHandle_t medicion_task_handle = NULL;

/** @var dato_tecla
 *  @brief Variable que almacena el dato leído del teclado.
 */
uint8_t dato_tecla;

/** @var string
 *  @brief Buffer para almacenar la cadena generada con la distancia y la unidad.
 */
const char string[11];

/** @var unidad
 *  @brief Cadena que representa la unidad de medida utilizada, en este caso "cm".
 */
const char unidad[3] = "cm";

/** @var inCm
 *  @brief Indica si la medición está en centímetros.
 */
bool inCm = true;

/** @var inIn
 *  @brief Indica si la medición está en pulgadas.
 */
bool inIn = false;

/*==================[internal data definition]===============================*/
/*==================[internal functions declaration]=========================*/

/**
 * @brief Alterna el estado del switch 1.
 * @details Cambia el estado de encendido/apagado para controlar la pantalla.
 */
static void evento_switch1(){
    estadoS1 = !estadoS1;
}

/**
 * @brief Alterna el estado del switch 2.
 * @details Cambia el estado de activación/desactivación para controlar la medición.
 */
static void evento_switch2(){
    estadoS2 = !estadoS2;
}

/**
 * @brief Función que notifica a la tarea de la pantalla.
 * @details Se llama desde una interrupción para notificar a la tarea que maneja la pantalla.
 */
void funcTimerPantalla(){
    vTaskNotifyGiveFromISR(pantalla_task_handle, pdFALSE);
}

/**
 * @brief Función que notifica a la tarea de medición.
 * @details Se llama desde una interrupción para notificar a la tarea que realiza la medición de distancia.
 */
void funcTimerMedir(){
    vTaskNotifyGiveFromISR(medicion_task_handle, pdFALSE);
}

/**
 * @brief Función para medir la distancia utilizando el sensor HcSr04.
 * @param[in] pvParameter Parámetro no utilizado.
 */
static void medirDistancia(void *pvParameter){
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (estadoS2 == ON) {
            // Si el switch 2 está encendido, no se mide la distancia
        } else {
            distancia = HcSr04ReadDistanceInCentimeters();
        }
    }
}

/**
 * @brief Función para mostrar la distancia medida en la pantalla LCD y manejo de LEDs.
 * @param[in] pvParameter Parámetro no utilizado.
 */
static void mostrarDistancia(void *pvParameter){
    while(1){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (estadoS1 == OFF) {
            LcdItsE0803Write(distancia);
            UartSendString(UART_PC, (char*)UartItoa(distancia, 10));
            UartSendString(UART_PC, " ");
            UartSendString(UART_PC, unidad);
            UartSendString(UART_PC, "\n");

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
 * @brief Maneja la entrada de datos del teclado.
 * @details Lee un byte desde el puerto serie y cambia el estado de los switches según el valor recibido.
 */
void uart_in(){
    UartReadByte(UART_PC, &dato_tecla);
    switch (dato_tecla){
        case 'O':
        case 'o':
            estadoS1 = !estadoS1;
            break;

        case 'H':
        case 'h':
            estadoS2 = !estadoS2;
            break;
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

    // Inicializacion de UART
    serial_config_t uart = {
        .port = UART_PC,
        .baud_rate = 9600,
        .func_p = uart_in,
        .param_p = NULL
    };
    UartInit(&uart);

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