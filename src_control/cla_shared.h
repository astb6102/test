#ifndef _CLA_SHARED_H_
#define _CLA_SHARED_H_

#include <stdint.h>

// CPU에서 CLA로 (필요 시 트리거 확인용)
typedef struct {
    uint16_t dummy; 
} CPU_TO_CLA_T;

// CLA에서 CPU로 (엔코더 읽기 결과물)
typedef struct {
    uint32_t encoder_raw;      // SPI에서 읽은 원시 비트 값
    float    position_deg;     // 가공된 기계각 (0 ~ 360도)
    uint32_t isr_tick;
} CLA_TO_CPU_T;

extern CPU_TO_CLA_T CpuToCla;
extern CLA_TO_CPU_T ClaToCpu;

#endif