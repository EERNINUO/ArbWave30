#include "calibration.h"
#include <stdbool.h>
#include "analog_board_driver.h"

#define CAL_NUM_POINTS 18

const uint32_t cal_freq_points[CAL_NUM_POINTS] = {
    10, 40, 70, 100, 1000, 10000, 100000,
    1000000, 3000000, 5000000, 8000000, 10000000, 14000000, 
    17000000, 20000000, 24000000, 27000000, 30000000
};

typedef struct {
    uint16_t cal_gain_points[CAL_NUM_POINTS];
    bool cal_valid;
} CalTable_t;

static CalTable_t cal_table[2] = {
    {
        {44900, 45182, 45017, 44915, 44740, 44617, 44664, 44595, 44714, 44976, 45506, 46084, 46965, 48222, 49170, 51311}, 
        true 
    },
    {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        false 
    }
}; 

static int16_t apply_gain(int16_t ideal_mV, int32_t gain)
{
    // 实际值 = 理想值 × (1 + gain/10000)
    int32_t result = (int32_t)ideal_mV * gain;
    if (result > INT16_MAX) result = INT16_MAX;
    if (result < INT16_MIN) result = INT16_MIN;
    return (int16_t)result;
}

int16_t calibration(uint8_t channel, uint64_t freq_hz, int16_t ideal_mV)
{
    CalTable_t *table = &cal_table[channel - 1];
    
    if (!table->cal_valid) {
        return ideal_mV;  // 无校准数据，直接返回
    }
    
    // 在固定频率点数组中查找区间
    int i;
    if (freq_hz <= cal_freq_points[0]) {
        // 低于最低校准点，用第一个点的增益
        return apply_gain(ideal_mV, table->cal_gain_points[0]);
    }
    if (freq_hz >= cal_freq_points[CAL_NUM_POINTS-1]) {
        // 高于最高校准点，用最后一个点的增益
        return apply_gain(ideal_mV, table->cal_gain_points[CAL_NUM_POINTS-1]);
    }
    
    for (i = 0; i < CAL_NUM_POINTS - 1; i++) {
        if (freq_hz >= cal_freq_points[i] && freq_hz <= cal_freq_points[i+1]) {
            // 线性插值
            uint32_t f0 = cal_freq_points[i];
            uint32_t f1 = cal_freq_points[i+1];
            int32_t g0 = table -> cal_gain_points[i];
            int32_t g1 = table -> cal_gain_points[i+1];
            
            int32_t gain = g0 + ((int32_t)((int64_t)(freq_hz - f0) * (g1 - g0) / (f1 - f0)) >> 16);
            return apply_gain(ideal_mV, gain);
        }
    }
    
    return ideal_mV;  // 理论上不会走到这里
}