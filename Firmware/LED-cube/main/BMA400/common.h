#ifndef __BMA400_COMMON_H__
#define __BMA400_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

#include "bma400.h"

int8_t bma400_interface_init(struct bma400_dev *bme, uint8_t intf);

int8_t bma400_read_i2c(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);

int8_t bma400_write_i2c(uint8_t i2c_addr, uint8_t reg_addr, const uint8_t *reg_data, uint16_t length);

void bma400_delay_us(uint32_t period, void *intf_ptr);

void bma400_check_rslt(const char api_name[], int8_t rslt);

void bma400_coines_deinit(void);

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif // __BMA400_COMMON_H__
