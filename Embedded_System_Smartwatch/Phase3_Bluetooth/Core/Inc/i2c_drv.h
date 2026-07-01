#ifndef I2C_DRV_H
#define I2C_DRV_H
#include "stm32f1xx_hal.h"

HAL_StatusTypeDef I2C1_MasterTransmit(uint8_t devAddr, uint8_t *pData, uint16_t len);
HAL_StatusTypeDef I2C2_WriteReg(uint8_t devAddr, uint8_t regAddr, uint8_t *pData, uint16_t len);
HAL_StatusTypeDef I2C2_ReadReg(uint8_t devAddr, uint8_t regAddr, uint8_t *pData, uint16_t len);

void I2C1_RecoverBus(void);
void I2C2_RecoverBus(void);

#endif