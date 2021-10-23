#include <adi_processor.h>
#include <pwr/adi_pwr.h>
#include <gpio/adi_gpio.h>

uint32_t gpio_mem[ADI_GPIO_MEMORY_SIZE];

int main(void)
{
    adi_pwr_Init();
    adi_pwr_SetClockDivider(ADI_CLOCK_HCLK, 1);
    adi_pwr_SetClockDivider(ADI_CLOCK_PCLK, 1);
    
    adi_gpio_Init(gpio_mem, ADI_GPIO_MEMORY_SIZE);
    adi_gpio_OutputEnable(ADI_GPIO_PORT2, ADI_GPIO_PIN_8, 1);
    
    // GPIO42 (P2_10), GPIO40 (P2_08) - LED st. 39 MCU manuálu
    while (1) {
        adi_gpio_SetHigh(ADI_GPIO_PORT2, ADI_GPIO_PIN_8);
        for (volatile int i = 0; i < 10000000; i++)
            ;
        adi_gpio_SetLow(ADI_GPIO_PORT2, ADI_GPIO_PIN_8);
        for (volatile int i = 0; i < 10000000; i++)
            ;
    }
    
    adi_gpio_UnInit();
}


/*
 * MCU: ADuCM4050 (ARM Cortex-M4F) w/Flash: 512 kB, SRAM: 128 kB, 12-bit SAR ADC (1.8 MSPS, 8 ch)
 * iCOMOX SMIP
 * Vibration sensor: ADXL356 (A2D interface)
 * Low-power vibration sensor: ADXL362 (SPI interface)
 * SmartMesh IP (2.4 GHz, 802.15.4e): LTC5800-IPM  (UART interface)
 * microSD (SPI inteface)
 * 
 * Components:
 * Network receive: Rules and Queries
 * Network transmit: Result sets
 * Read bus: Accelerometers
 * Processing: Act on queries - Extract frequencies, Compress data
 * Logging: micro SD card
 */


