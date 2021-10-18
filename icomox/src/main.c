#include <adi_processor.h>
#include <pwr/adi_pwr.h>
#include <gpio/adi_gpio.h>
#include <spi/adi_spi.h>
#include <uart/adi_uart.h>
#include <arm_math.h>
#include <FreeRTOS.h>

#define IDLE_TASK_SIZE 1000

// https://mcuoneclipse.com/2016/05/29/freertos-v9-0-0-with-static-memory-allocation/

#if configSUPPORT_STATIC_ALLOCATION
/* static memory allocation for the IDLE task */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[IDLE_TASK_SIZE];
 
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = IDLE_TASK_SIZE;
}
#endif

#if configSUPPORT_STATIC_ALLOCATION && configUSE_TIMERS
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];
 
/* If static allocation is supported then the application must provide the
   following callback function - which enables the application to optionally
   provide the memory that will be used by the timer task as the task's stack
   and TCB. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif


int main(void)
{
    // MCU: ADuCM4050 (ARM Cortex-M4F) w/Flash: 512 kB, SRAM: 128 kB, 12-bit SAR ADC (1.8 MSPS, 8 ch)
    // iCOMOX SMIP
    // Vibration sensor: ADXL356 (A2D interface)
    // Low-power vibration sensor: ADXL362 (SPI interface)
    // SmartMesh IP (2.4 GHz, 802.15.4e): LTC5800-IPM  (UART interface)
    // microSD (SPI inteface)
    
    // Components:
    // Network receive: Rules and Queries
    // Network transmit: Result sets
    // Read bus: Accelerometers
    // Processing: Act on queries - Extract frequencies, Compress data
    // Logging: micro SD card
    
    while (1)
        ;
}


