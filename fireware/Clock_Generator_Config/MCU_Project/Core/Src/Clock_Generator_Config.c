#include "Clock_Generator_Config.h"

// 配置表中的条目数
#define CDCI6214_CONFIG_SIZE  (sizeof(cdci6214_regs) / sizeof(cdci6214_regs[0]))

/**
 * @brief 通过I2C将配置写入CDCI6214
 * @param hi2c I2C句柄
 * @param dev_addr 设备7位地址 (例如0x74)
 * @return HAL状态
 */
HAL_StatusTypeDef CDCI6214_WriteConfig(I2C_HandleTypeDef *hi2c, uint8_t dev_addr)
{
    HAL_StatusTypeDef status = HAL_OK;

    for (uint32_t i = 0; i < CDCI6214_CONFIG_SIZE; i++)
    {
        // 使用HAL_I2C_Mem_Write写入16位寄存器地址和16位数据
        // 参数: hi2c, 设备地址, 寄存器地址(16位), 地址长度, 数据指针, 数据长度(字节), 超时
        status = HAL_I2C_Mem_Write (hi2c,
                                    dev_addr,
                                    (uint16_t)cdci6214_regs[i].addr,
                                    I2C_MEMADD_SIZE_16BIT,
                                    (uint8_t*)&cdci6214_regs[i].value,
                                    2,  // 写入2字节 (16位数据)
                                    HAL_MAX_DELAY);  // 或使用合适的超时时间，如1000

        if (status != HAL_OK)
        {
            // 写入失败，可在此添加错误处理（如返回错误码或重试）
            return status;
        }

        // 可选：短暂延时，确保器件处理完成。CDCI6214速度较快，通常不需要。
        // HAL_Delay(1);
    }

    return HAL_OK;
}