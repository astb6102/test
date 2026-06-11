// cla_shared.h
#ifndef _CLA_SHARED_H_
#define _CLA_SHARED_H_

#include "driverlib.h"
#include "device.h"

// CPU -> CLA 방향 데이터 (지령값, 제어기 게인 등)
typedef struct {
    float Iq_ref;        // Q축 전류 지령값
    float Id_ref;        // D축 전류 지령값
    float Kp_Id;         
    float Ki_Id;         
    float Kp_Iq;         
    float Ki_Iq;         
   float sin_theta;  // 추가: CPU에서 계산해서 넘겨줄 sin 값
    float cos_theta;  // 추가: CPU에서 계산해서 넘겨줄 cos 값
    float I_scale;       // 전류 스케일 (USER_M1_CURRENT_SF * USER_M1_SIGN_CURRENT_SF)
    float I_offset_U;    // U상 오프셋 (예: 2048.0f 근처)
    float I_offset_V;    // V상 오프셋 (예: 2048.0f 근처)
    float Vdc_bus;       // DC 링크 버스 전압 계측값
    uint16_t pwm_period; // 실제 PWM 주기 (USER_M1_PWM_TBPRD_NUM)
} CPU_TO_CLA_VARS;

// CLA -> CPU 방향 데이터 (모니터링, 보호 계측용)
typedef struct {
    float I_u;          // U상 전류 측정값
    float I_v;          // V상 전류 측정값
    float I_w;          // W상 전류 측정값
    float Id_meas;      // D축 전류 실제값
    float Iq_meas;      // Q축 전류 실제값
    uint32_t isr_tick;  // 동작 확인용 카운터
} CLA_TO_CPU_VARS;

extern CPU_TO_CLA_VARS CpuToCla;
extern CLA_TO_CPU_VARS ClaToCpu;

#endif // _CLA_SHARED_H_