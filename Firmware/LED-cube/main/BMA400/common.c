#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "bma400.h"

/* Variable to store the device address */
static uint8_t dev_addr;

BMA400_INTF_RET_TYPE bma400_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{

}

BMA400_INTF_RET_TYPE bma400_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{

}

void bma400_delay_us(uint32_t period, void *intf_ptr)
{
    // Looking at the use cases this function is never called with a value <1000
    // A us delay is a pain so just use ms delay and have a minimum 1 ms
    if (period < 1000)
    {
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    else 
    {
        vTaskDelay(period / (1000 * portTICK_PERIOD_MS));
    }
}

void bma400_check_rslt(const char api_name[], int8_t rslt)
{
    switch (rslt)
    {
        case BMA400_OK:
            /* Do nothing */
            break;
        case BMA400_E_NULL_PTR:
            printf("API : %s Error [%d] : Null pointer\r\n", api_name, rslt);
            break;
        case BMA400_E_COM_FAIL:
            printf("API : %s Error [%d] : Communication failure\r\n", api_name, rslt);
            break;
        case BMA400_E_INVALID_CONFIG:
            printf("API : %s Error [%d] : Invalid configuration\r\n", api_name, rslt);
            break;
        case BMA400_E_DEV_NOT_FOUND:
            printf("API : %s Error [%d] : Device not found\r\n", api_name, rslt);
            break;
        default:
            printf("API : %s Error [%d] : Unknown error code\r\n", api_name, rslt);
            break;
    }
}

int8_t bma400_interface_init(struct bma400_dev *bma400, uint8_t intf)
{

}

void bma400_coines_deinit(void)
{

}
