
//! \file   /solutions/universal_motorcontrol_lab/common/source/sys_main.c
//!
//! \brief  This project is used to implement motor control with FAST, eSMO
//!         Encoder, and Hall sensors based sensored/sensorless-FOC.
//!         Supports multiple TI EVM boards
//!
//
// dcsm_security_tool guidance refer to: http://www.ti.com/lit/pdf/spracp8
//
//

// include the related header files
//
#include "user.h"
#include "sys_settings.h"
#include "sys_main.h"
#include "motor_common.h"
#include "driverlib.h"
#include "device.h"           // 디바이스 관련 기본 정의 (MCU별 다름)
#include "inc/hw_memmap.h"      // 베이스 주소 정의 포함
#include "i2c.h"
//#include <ti/drivers/I2C.h>
#include <stdbool.h>

#include "driverlib/mcan.h"

#define I2C_BUSY_TIMEOUT 100000
#define I2C_STOP_TIMEOUT 100000
#define I2C_RXFIFO_TIMEOUT 100000
#define I2C_TIMEOUT_COUNT 10000
extern void updateCANSetupParams(void);
extern uint32_t Cla1ProgRunStart, Cla1ProgLoadStart, Cla1ProgLoadSize;

volatile SYSTEM_Vars_t systemVars;
//uint16_t position_enc; syh global
float temp_off_a=2.0948f;
float eeprom_r;
bool wc_s=FALSE;
int eep_read=0,eep_write=0,addr=0;
float ab,cd;
uint8_t buf[4],temp_data;
uint8_t g_float_buffer[4];
bool IG_IN;
bool IG_OUT;
// 송수신 헤더 및 데이터 버퍼
uint8_t CAN_nINT;
uint8_t txData[8] = {0};
uint8_t rxData[8] = {0}; // Classic CAN은 최대 8바이트

uint8_t B_CAN_RX_H[4];
uint8_t B_CAN_RX_L[4];

uint8_t B_CAN_RX[8];
uint8_t B_CAN_TX[8];
uint8_t B_CAN_TX_H[4];
uint8_t B_CAN_TX_L[4];
uint16_t tx[4] = {0x4100, 0x0001, 0x0000, 0x0000};
uint16_t rx[4];
uint16_t tx_TEST[4] = {0x4100, 0x0001, 0x0000, 0x0000};
uint16_t rx_TEST[4];
uint8_t TEST_CHECK;
uint16_t tx2[4];
uint16_t rx2[8];
uint32_t TX_BUFFER[2] = {0x11223344, 0x55667788};
uint32_t rx_data[2];
uint32_t tx_data[2]= {0x11223344, 0x55667788};
uint8_t test_CAN_B;
uint16_t check_rx3;
volatile uint64_t readWord;
uint8_t SPI_READ;
int testtest;
int MOTOR_RUN;
int test1;
float Voffset;
int test;
int SPI_TEST;
uint16_t SPI_DATA;
int TCAN_SEND_TEST = 0;
uint32_t TCAN_DATA_H = 0x11223344;
uint32_t TCAN_DATA_L = 0x55667788;
uint8_t clear_Test;
uint32_t test_time;
uint32_t variabe_time;
uint8_t CAN_B_CHECK;
uint8_t rx1;
uint8_t tx1;
int reset=0;
uint8_t Case_pick=0;
int case_selection;
float32_t boardTemp,boardHumi,motorTemp;
const float V_SUPPLY = 5.0f;
    const float R_PULLDOWN = 1500.0f;

    const float R0 = 1000.0f;
    const float A  = 3.9083e-3f;
    const float B  = -5.775e-7f;
    const float C  = -4.183e-12f;

    volatile bool g_flagEepromSaveReq = false;
    volatile uint8_t g_eepromSaveIndex = 0;
    volatile float g_eepromSaveValue = 0.0f;
    volatile bool g_flagEepromSaveAllReq = false; // 일괄 저장 플래그

// [추가 변수] TCAN4550 16바이트 통신용 버퍼
uint16_t rxTemp16[16];  // 16워드 수신 임시버퍼
uint16_t txTemp16[16];  // 16워드 송신 임시버퍼
uint8_t  rx16[8];       // TCAN 최종 8바이트 응답 데이터
uint8_t tx_flag1;
uint8_t clear_flag;
uint8_t tx_build_flag;
// [매크로] _INLINE 링커 에러 방지용 SPI/GPIO 직접 접근 매크로
#define SPI_CCR_REG         (SPIA_BASE + 0x00U)  // SPICCR (모드 변경용)
#define SPI_TXBUF_REG       (SPIA_BASE + 0x06U)
#define SPI_RXBUF_REG       (SPIA_BASE + 0x07U)
#define SPI_FFTX_REG        (SPIA_BASE + 0x08U)
#define SPI_FFRX_REG        (SPIA_BASE + 0x09U)

#define SPI_TX_FIFO_FULL()  ((HWREGH(SPI_FFTX_REG) & 0x1F00U) == 0x1000U)
#define SPI_RX_FIFO_EMPTY() ((HWREGH(SPI_FFRX_REG) & 0x1F00U) == 0x0000U)
//#define GPIO_SET_PIN(pin)   (HWREGH(GPASET)   |=  (1U << (pin)))
//#define GPIO_CLR_PIN(pin)   (HWREGH(GPACLEAR) |= (1U << (pin)))
typedef union {
    float    f;
    uint8_t  bytes[4];
} FloatByteUnion;

FloatByteUnion g_converter;
FloatByteUnion g_converter_read;
//uint8_t ptr[4];
// M24C08-DRMN3TP EEPROM I2C 슬레이브 주소 (A2, A1 핀이 GND에 연결된 경우)
#define EEPROM_SLAVE_ADDR       0x50

// EEPROM 페이지 크기 (바이트)
#define EEPROM_PAGE_SIZE        16

#pragma DATA_SECTION(systemVars,"sys_data");

#ifdef CPUTIME_ENABLE
// define CPU time for performance test
CPU_TIME_Obj     cpuTime;
CPU_TIME_Handle  cpuTimeHandle;
//I2C_Handle         i2cHandle;
#pragma DATA_SECTION(cpuTime,"sys_data");
#pragma DATA_SECTION(cpuTimeHandle,"sys_data");
#endif  // CPUTIME_ENABLE


#if defined(EPWMDAC_MODE)
#if defined(HVMTRPFC_REV1P1)
HAL_PWMDACData_t pwmDACData;
#pragma DATA_SECTION(pwmDACData,"sys_data");
  // HVMTRPFC_REV1P1
#else
#error EPWMDAC is not supported on this kit!
#endif  // !HVMTRPFC_REV1P1
#endif  // EPWMDAC_MODE

#if defined(DAC128S_ENABLE)
DAC128S_Handle   dac128sHandle;        //!< the DAC128S interface handle
DAC128S_Obj      dac128s;              //!< the DAC128S interface object
#pragma DATA_SECTION(dac128sHandle,"sys_data");
#pragma DATA_SECTION(dac128s,"sys_data");

#define DAC_SCALE_SET       (4096.0f)     // 12bit
#endif  // DAC128S_ENABLE
//test code
//


#if defined(SFRA_ENABLE)
float32_t   sfraNoiseId;
float32_t   sfraNoiseIq;
float32_t   sfraNoiseSpd;
float32_t   sfraNoiseOut;
float32_t   sfraNoiseFdb;
SFRA_TEST_e sfraTestLoop;        //speedLoop;
bool        sfraCollectStart;

#pragma DATA_SECTION(sfraNoiseId, "SFRA_F32_Data");
#pragma DATA_SECTION(sfraNoiseIq, "SFRA_F32_Data");
#pragma DATA_SECTION(sfraNoiseSpd, "SFRA_F32_Data");
#pragma DATA_SECTION(sfraNoiseOut, "SFRA_F32_Data");
#pragma DATA_SECTION(sfraNoiseFdb, "SFRA_F32_Data");
#pragma DATA_SECTION(sfraTestLoop, "SFRA_F32_Data");
#pragma DATA_SECTION(sfraCollectStart, "SFRA_F32_Data");
#endif  // SFRA_ENABLE

// **************************************************************************
// the functions
// !!! Please make sure that you had gone through the user guide, and follow the
// !!! guide to set up the kit and load the right code
// 함수 프로토타입
void InitI2C(void);
void EEPROM_writeByte(uint16_t memAddr, uint8_t data);
uint8_t EEPROM_readByte(uint16_t memAddr);
void EEPROM_writePage(uint16_t memAddr, uint8_t* data, uint16_t dataLen);
void EEPROM_read(uint16_t memAddr, uint8_t* data, uint16_t dataLen);
static void EEPROM_waitForWriteCompletion(void);

// I2C 초기화 함수
void InitI2C(void)
{
   // I2C 모듈 리셋
    I2C_disableModule(I2CB_BASE);

    // I2C 컨트롤러 모드로 초기화 (400kHz)
    I2C_initController(I2CB_BASE, DEVICE_SYSCLK_FREQ, 400000, I2C_DUTYCYCLE_50);

    // FIFO 비활성화 (이전에 제공된 EEPROM 함수와 호환성을 위해)
    I2C_disableFIFO(I2CB_BASE);

    // 7비트 주소 모드 설정
    I2C_setAddressMode(I2CB_BASE, I2C_ADDR_MODE_7BITS);

    // 에뮬레이션 설정
    I2C_setEmulationMode(I2CB_BASE, I2C_EMULATION_FREE_RUN);

    // I2C 모듈 활성화
    I2C_enableModule(I2CB_BASE);
}
// I2C 초기화 함수
 void SPI_TX_build(uint8_t rw, uint16_t addr, uint8_t len, uint32_t data)
{
    tx[0] = ((rw ? 0x61 : 0x41) << 8) |((addr >> 8) & 0xFF);
    tx[1] = ((addr & 0xFF) << 8) | (len & 0xFF);
    tx[2] = (data >> 16) & 0xFFFF;
    tx[3] = (data >> 0) & 0xFFFF;
}
 void SPI_TX_TEST_build(uint8_t rw, uint16_t addr, uint8_t len, uint32_t data)
{
    tx[0] = ((rw ? 0x61 : 0x41) << 8) |((addr >> 8) & 0xFF);
    tx[1] = ((addr & 0xFF) << 8) | (len & 0xFF);
    tx[2] = (data >> 16) & 0xFFFF;
    tx[3] = (data >> 0) & 0xFFFF;
}

void SPI_TX_build2(uint8_t rw, uint16_t addr, uint8_t len, uint32_t *data)
{
    tx2[0] = ((rw ? 0x61 : 0x41) << 8) | ((addr >> 8) & 0xFF);
    tx2[1] = ((addr & 0xFF) << 8) | (len & 0xFF);
    
    uint8_t i;

    for (i = 0; i < len; i++)
    {
        tx2[2 + (i * 2)]     = (data[i] >> 16) & 0xFFFF;  // MSB
        tx2[2 + (i * 2) + 1] = (data[i] >> 0)  & 0xFFFF;  // LSB
    }
}
void SPI_OnceTransaction_TEST(void)
{
 //   while (SPI_getRxFIFOStatus(SPIA_BASE) != SPI_FIFO_RX0)
 //   {
//        SPI_readDataNonBlocking(SPIA_BASE);
//    }

    GPIO_writePin(27, 0);
  //  SPI_resetRxFIFO(SPIA_BASE);
  //  SPI_resetTxFIFO(SPIA_BASE);
 //   SPI_clearInterruptStatus(SPIA_BASE, SPI_INT_RXFF);
    SPI_pollingFIFOTransaction(SPIA_BASE, 16U, tx_TEST, rx_TEST, 4U, 0U);
    GPIO_writePin(27, 1); // CS HIGH

}
void SPI_OnceTransaction(void)
{
 //   while (SPI_getRxFIFOStatus(SPIA_BASE) != SPI_FIFO_RX0)
 //   {
//        SPI_readDataNonBlocking(SPIA_BASE);
//    }
DINT;
    GPIO_writePin(27, 0);
  //  SPI_resetRxFIFO(SPIA_BASE);
  //  SPI_resetTxFIFO(SPIA_BASE);
 //   SPI_clearInterruptStatus(SPIA_BASE, SPI_INT_RXFF);
     
    SPI_pollingFIFOTransaction(SPIA_BASE, 16U, tx, rx, 4U, 0U);
   
    GPIO_writePin(27, 1); // CS HIGH
     EINT;

}
void SPI_OnceTransaction2(void)
{
    DINT;
    GPIO_writePin(27, 0);
   // SPI_resetRxFIFO(SPIA_BASE);
  //  SPI_resetTxFIFO(SPIA_BASE);
  //  SPI_clearInterruptStatus(SPIA_BASE, SPI_INT_RXFF);
    SPI_pollingFIFOTransaction(SPIA_BASE, 16U, tx2, rx2, 8U, 0U);
    GPIO_writePin(27, 1); // CS HIGH
    EINT;
}

void B_CAN_Interrupt_Check(void)
{
    SPI_TX_build(0, 0x1050, 1, 0x00000000);
    DINT;
    SPI_OnceTransaction(); 
    EINT;
}

void B_CAN_Interrupt_Check_test(void)
{
    SPI_TX_build(0, 0x0000, 1, 0x00000000);
    DINT;
    GPIO_writePin(27, 0);
    SPI_pollingFIFOTransaction(SPIA_BASE, 16U, tx, rx, 4U, 0U);
    GPIO_writePin(27, 1);
    EINT;
    //SPI_OnceTransaction(); 
}



void SPI_InitSequence(void)
{
     static uint8_t once = 0;
    if (once) return;
    once = 1;
    // 1. SPI 통신 안정화 (더미 읽기)
    SPI_TX_build(0, 0x0000, 1, 0x00000000);
    SPI_OnceTransaction();

    SPI_TX_build(1, 0x0820, 1, 0xFFFFFFFF); // 디바이스 인터럽트 클리어
    SPI_OnceTransaction();
    SPI_TX_build(1, 0x0824, 1, 0xFFFFFFFF); // MCAN 인터럽트 클리어
    SPI_OnceTransaction();

    SPI_TX_build(1, 0x1018, 1, 0x00000003);  //초기화 모드
    SPI_OnceTransaction();

    SPI_TX_build(1, 0x101C, 1, 0x02030F02);  // 통신속도 설정
    SPI_OnceTransaction();

    SPI_TX_build(1, 0x10C0, 1, 0x00010000);   // 쓰기주소 8000
    SPI_OnceTransaction();

    SPI_TX_build(1, 0x10A0, 1, 0x80010400);   // 읽기주소 8400
    SPI_OnceTransaction();

    SPI_TX_build(1, 0x1080, 1, 0x0000002A);   // ID랑 안맞는 거 어떻게 할지 현재는 버림
    SPI_OnceTransaction();

    SPI_TX_build(1, 0x1084, 1, 0x00010200);   // ID 갯수 및 ID 주소 설정
    SPI_OnceTransaction();

    SPI_TX_build(1, 0x8200, 1, 0x880107FF);   // 아이디 및 마스크 설정/ 맞는 아이디면 fifo0에 저장
    SPI_OnceTransaction();

    SPI_TX_build(1, 0x10BC, 1, 0x00000000);   //데이터 필드 사이즈
    SPI_OnceTransaction();

    SPI_TX_build(1, 0x8000, 1, 0x00080000);  // ID 설정 (예: Standard ID 0x001)
    SPI_OnceTransaction();

    SPI_TX_build(1, 0x8004, 1, 0x00080000);  // DLC 8
    SPI_OnceTransaction();

    SPI_TX_build(1, 0x0800, 1, 0xC8000483);   // 노말 모드 설정
    SPI_OnceTransaction();

    SPI_TX_build(1, 0x1018, 1, 0x00000000);   // 초기화 모드 해제
    SPI_OnceTransaction();

 ///////////////////////MCAN Interrupt Flags,Interrupt Flags 초기화///////////////////
    SPI_TX_build(1, 0x0820, 1, 0xFFFFFFFF);
    SPI_OnceTransaction();
    SPI_TX_build(1, 0x0824, 1, 0xFFFFFFFF);
    SPI_OnceTransaction();
    ////////////////////////////////////////////////////////////////////////////////
}
static bool waitForTxReady(void)
{
    uint32_t timeout = I2C_TIMEOUT_COUNT;

    // ARDY(Access Ready) 비트는 전송이 완료되고 FIFO에 접근 가능할 때 1이 됩니다.
    // 이 비트가 1이 될 때까지 기다립니다.
    while((I2C_getStatus(I2CB_BASE) & I2C_STR_ARDY) == 0)
    {
        if(--timeout == 0)
        {
            // 타임아웃 발생. I2C를 리셋하는 등의 에러 복구 로직이 필요할 수 있습니다.
            return false;
        }
    }
    return true;
}

uint8_t spi_transfer(uint8_t data)
{
    SPI_writeDataBlockingNonFIFO(SPIA_BASE, data);
    return (uint8_t)SPI_readDataBlockingNonFIFO(SPIA_BASE);
}


void EEPROM_writeByte(uint8_t memAddr, uint8_t data)
{
    while(I2C_isBusBusy(I2CB_BASE));
    I2C_setTargetAddress(I2CB_BASE, 0x50);
    I2C_setDataCount(I2CB_BASE, 2); // 주소 1바이트 + 데이터 1바이트
    I2C_setConfig(I2CB_BASE, I2C_CONTROLLER_SEND_MODE);
    I2C_sendStartCondition(I2CB_BASE);

    I2C_putData(I2CB_BASE, memAddr); // 내부주소(1바이트)
    DEVICE_DELAY_US(1000);
    I2C_putData(I2CB_BASE, data); // 데이터 바이트
    DEVICE_DELAY_US(1000);

    I2C_sendStopCondition(I2CB_BASE); // Stop
    DEVICE_DELAY_US(5000);
    // 내부 write 완료 대기 필요 시 이곳에서 ACK polling 수행
}

uint8_t EEPROM_readByte(uint8_t memAddr)
{
        uint8_t data;

    // 주소 지정(Dummy Write)
    while(I2C_isBusBusy(I2CB_BASE));
    I2C_setTargetAddress(I2CB_BASE, 0x50);
    I2C_setDataCount(I2CB_BASE, 1);    // 주소 1바이트만
    I2C_setConfig(I2CB_BASE, I2C_CONTROLLER_SEND_MODE);
    I2C_sendStartCondition(I2CB_BASE);
    I2C_putData(I2CB_BASE, memAddr);
    DEVICE_DELAY_US(1000);
    I2C_sendStopCondition(I2CB_BASE);
    while(I2C_isBusBusy(I2CB_BASE));
    
    // 데이터 읽기(Read)
    I2C_setTargetAddress(I2CB_BASE, 0x50); // Read 모드
    I2C_setDataCount(I2CB_BASE, 1);        // 데이터 1바이트
    I2C_setConfig(I2CB_BASE, I2C_CONTROLLER_RECEIVE_MODE);
    I2C_sendStartCondition(I2CB_BASE); 
    DEVICE_DELAY_US(1000);
    data = I2C_getData(I2CB_BASE);
    I2C_sendStopCondition(I2CB_BASE);

    return data;
}

// EEPROM 페이지 쓰기 함수 (최대 16바이트)
void EEPROM_writePage(uint16_t memAddr, uint8_t* data, uint16_t dataLen)
{
    uint16_t i;
    if (dataLen == 0 || dataLen > EEPROM_PAGE_SIZE) return;

    while(I2C_isBusBusy(I2CB_BASE));
    I2C_setTargetAddress(I2CB_BASE, EEPROM_SLAVE_ADDR);
    I2C_setDataCount(I2CB_BASE, dataLen + 2);
    I2C_setConfig(I2CB_BASE, I2C_CONTROLLER_SEND_MODE);
    I2C_sendStartCondition(I2CB_BASE);

    I2C_putData(I2CB_BASE, (uint8_t)(memAddr >> 8));
    while((I2C_getStatus(I2CB_BASE) & I2C_STS_TX_DATA_RDY) == 0);

    I2C_putData(I2CB_BASE, (uint8_t)(memAddr & 0xFF));
    while((I2C_getStatus(I2CB_BASE) & I2C_STS_TX_DATA_RDY) == 0);

    for (i = 0; i < dataLen; i++)
    {
        I2C_putData(I2CB_BASE, data[i]);
        while((I2C_getStatus(I2CB_BASE) & I2C_STS_TX_DATA_RDY) == 0);
    }

    I2C_sendStopCondition(I2CB_BASE);
    EEPROM_waitForWriteCompletion();
}

// EEPROM 연속 읽기 함수
void EEPROM_read(uint16_t memAddr, uint8_t* data, uint16_t dataLen)
{
    uint16_t i;
    if (dataLen == 0) return;

    // --- Dummy Write ---
    while(I2C_isBusBusy(I2CB_BASE));
        I2C_setTargetAddress(I2CB_BASE, EEPROM_SLAVE_ADDR);
        I2C_setDataCount(I2CB_BASE, 2);
        I2C_setConfig(I2CB_BASE, I2C_CONTROLLER_SEND_MODE);
        I2C_sendStartCondition(I2CB_BASE);
        I2C_putData(I2CB_BASE, (uint8_t)(memAddr >> 8));
    while((I2C_getStatus(I2CB_BASE) & I2C_STS_TX_DATA_RDY) == 0);
        I2C_putData(I2CB_BASE, (uint8_t)(memAddr & 0xFF));
    while((I2C_getStatus(I2CB_BASE) & I2C_STS_TX_DATA_RDY) == 0);
    while(I2C_isBusBusy(I2CB_BASE));

    // --- Read ---
        I2C_setTargetAddress(I2CB_BASE, EEPROM_SLAVE_ADDR);
        I2C_setDataCount(I2CB_BASE, dataLen);
        I2C_setConfig(I2CB_BASE, I2C_CONTROLLER_RECEIVE_MODE);
        I2C_sendStartCondition(I2CB_BASE); // Repeated Start

    for (i = 0; i < dataLen; i++)
    {
        while((I2C_getStatus(I2CB_BASE) & I2C_STS_RX_DATA_RDY) == 0);
        data[i] = (uint8_t)I2C_getData(I2CB_BASE);
    }

    I2C_sendStopCondition(I2CB_BASE);
}
void EEPROM_writeFloat(uint8_t memAddr, float value)
{
    // 1. union 변수를 선언합니다.
   // FloatByteUnion converter;
uint32_t int_value;
    // 2. union의 float 멤버에 값을 저장합니다.
    g_converter.f = value;
    memcpy(&int_value, &value, sizeof(value));

    // 3. 이제 converter.bytes 배열에는 float의 4바이트 표현이 들어있습니다.
    //    디버거에서 converter.bytes 배열의 내용을 직접 확인할 수 있습니다.
        g_converter.bytes[0] = (int_value >> 0)  & 0xFF; // 가장 낮은 바이트
        g_converter.bytes[1] = (int_value >> 8)  & 0xFF;
        g_converter.bytes[2] = (int_value >> 16) & 0xFF;
        g_converter.bytes[3] = (int_value >> 24) & 0xFF; // 가장 높은 바이트  



    int i;
    for(i = 0; i < 4; i++)
    {
        // 4. union의 바이트 배열을 EEPROM에 순차적으로 씁니다.
        EEPROM_writeByte(memAddr + i, g_converter.bytes[i]);
        DEVICE_DELAY_US(5000);
    }
}
float EEPROM_readFloat(uint8_t memAddr)
{
    // 1. union 변수를 선언합니다.
    //FloatByteUnion converter;

    int i;
    uint32_t int_value = 0;
    float result;

    for(i = 0; i < 4; i++)
    {
        // 2. EEPROM에서 읽은 바이트를 union의 바이트 배열에 순차적으로 저장합니다.
        g_converter_read.bytes[i] = EEPROM_readByte(memAddr + i);
    }
     //float EEPROM_readFloat(uint8_t memAddr)

 

    // 1. EEPROM에서 읽은 4개의 바이트를 비트 시프트 연산으로 하나의 32비트 정수로 재조립합니다.
    //    (리틀 엔디안 순서에 맞춰 조립)
    int_value |= (uint32_t)g_converter_read.bytes[0] << 0;
    int_value |= (uint32_t)g_converter_read.bytes[1] << 8;
    int_value |= (uint32_t)g_converter_read.bytes[2] << 16;
    int_value |= (uint32_t)g_converter_read.bytes[3] << 24;

    // 2. 재조립된 32비트 정수의 비트 패턴을 float 변수로 안전하게 복사하여 변환합니다.
    memcpy(&result, &int_value, sizeof(float));

    return result;

    // 3. 이제 union의 바f 멤버를 읽으면 자동으로 float 값으로 해석되어 반환됩니다.
  //  return g_converter.f;
}
// 인덱스(1~63)로 float 저장
void EEPROM_writeFloatByIndex(uint8_t floatIndex, float value)
{
    if(floatIndex < 1 || floatIndex > 63) return; // 범위 보호
    uint8_t memAddr = (floatIndex - 1) * 4;
    EEPROM_writeFloat(memAddr, value);
}

// 인덱스(1~63)로 float 읽기
float EEPROM_readFloatByIndex(uint8_t floatIndex)
{
    if(floatIndex < 1 || floatIndex > 63) return 0.0f; // 범위 보호
    uint8_t memAddr = (floatIndex - 1) * 4;
    return EEPROM_readFloat(memAddr);
}
// EEPROM 쓰기 동작 완료 대기 함수
static void EEPROM_waitForWriteCompletion(void)
{
    // EEPROM이 응답(ACK)할 때까지 슬레이브 주소를 반복적으로 전송 (폴링)
    while(1)
    {
        while(I2C_isBusBusy(I2CB_BASE));
        I2C_setTargetAddress(I2CB_BASE, EEPROM_SLAVE_ADDR);
        I2C_setConfig(I2CB_BASE, I2C_CONTROLLER_SEND_MODE);
        I2C_setDataCount(I2CB_BASE, 0); // 데이터 없이 주소만 확인
        I2C_sendStartCondition(I2CB_BASE);

        // NACK(응답 없음)가 아닐 때까지 대기
        // NACK가 발생하면 EEPROM이 아직 쓰기 중이라는 의미
        while((I2C_getStatus(I2CB_BASE) & I2C_STS_NO_ACK))
        {
             // 상태 클리어 및 재시도
            I2C_clearStatus(I2CB_BASE, I2C_STS_NO_ACK);
            I2C_sendStartCondition(I2CB_BASE);
        }

        // ACK가 수신되면 루프 탈출
        if (!(I2C_getStatus(I2CB_BASE) & I2C_STS_NO_ACK))
        {
            I2C_sendStopCondition(I2CB_BASE);
            break;
        }
    }
}
void main(void)
{
    // Clear memory for system and controller
    // The variables must be assigned to these sector if need to be cleared to zero
    HAL_clearDataRAM((void *)loadStart_est_data, (uint16_t)loadSize_est_data);
    HAL_clearDataRAM((void *)loadStart_user_data, (uint16_t)loadSize_user_data);
    HAL_clearDataRAM((void *)loadStart_hal_data, (uint16_t)loadSize_hal_data);
    HAL_clearDataRAM((void *)loadStart_foc_data, (uint16_t)loadSize_foc_data);
    HAL_clearDataRAM((void *)loadStart_sys_data, (uint16_t)loadSize_sys_data);
    HAL_clearDataRAM((void *)loadStart_vibc_data, (uint16_t)loadSize_vibc_data);
    HAL_clearDataRAM((void *)loadStart_datalog_data, (uint16_t)loadSize_datalog_data);
    HAL_clearDataRAM((void *)loadStart_SFRA_F32_Data, (uint16_t)loadSize_SFRA_F32_Data);
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_SCIB);//test 0725

    int enc_timer=0;
    int ci;
    motorVars_M1.position_control=0;
    motorVars_M1.position_ang=0;
    motorVars_M1.dead_zone=0.5;
    motorVars_M1.dead_zone_hys=0.1;
    motorVars_M1.dead_zone_flag=0;
    motorVars_M1.brake_hall_enable=0;
    motorVars_M1.pos_speed_lim=15;
    motorVars_M1.ffwdValue=1;
    
    motorVars_M1.Current_limit=1.0f;
    motorVars_M1.Torque_limit=1.0f;
  
    motorVars_M1.brake_ahall=0;
    motorVars_M1.est_ahall=0;
    motorVars_M1.temp_cal_offset=2.18;//2.0948;//2.2;//2.0948;real 

    motorVars_M1.temp_Kp=0.03f;//0.03->0.01
    motorVars_M1.temp_Ki=0.0005f;
    motorVars_M1.pos_Kp=5;//20.0f;//3->20
    motorVars_M1.pos_Ki=0.0f;
    motorVars_M1.posgain_Kp=0.5;//20.0f;//3->20
     // motorVars_M1.posgain_Ki=0.01;//20.0f;//3->20
    motorVars_M1.flagEnableMTPA=0;
    motorVars_M1.flagUpdateMTPAParams=0;
    motorVars_M1.flagEnableFWC=0;
    systemVars.gain_factor_kp_id=1.0f;// syh. def 1
    systemVars.gain_factor_ki_id=1.0f;// syh. def 1

#if defined(SYSCONFIG_EN)
    systemVars.projectConfig = PRJ_DEV_SYSCONFIG;
#else
    systemVars.projectConfig = PRJ_NON_SYSCONFIG;
#endif  // SYSCONFIG_EN

#if defined(HVMTRPFC_REV1P1)
    systemVars.boardKit = BOARD_HVMTRPFC_REV1P1;    // HVMTRPFC_REV1P1
#elif defined(DRV8329AEVM_REVA)
    systemVars.boardKit = BOARD_DRV8329AEVM_REVA;    // DRV8329AEVM_REVA
#elif defined(BSXL8323RS_REVA)
    systemVars.boardKit = BOARD_BSXL8323RS_REVA;    // BSXL8323RS_REVA
#elif defined(BSXL8323RH_REVB)
    systemVars.boardKit = BOARD_BSXL8323RH_REVB;    // BSXL8323RH_REVB
#elif defined(BSXL8353RS_REVA)
    systemVars.boardKit = BOARD_BSXL8353RS_REVA;    // BSXL8353RS_REVA
#elif defined(BSXL3PHGAN_REVA)
    systemVars.boardKit = BOARD_BSXL3PHGAN_REVA;    // BSXL3PHGAN_REVA
#elif defined(BSXL8316RT_REVA)
    systemVars.boardKit = BOARD_BSXL8316RT_REVA;    // BSXL8316RT_REVA
#elif defined(DRV8300DRGE_EVM)
    systemVars.boardKit = BOARD_BSXL8300RT_REVA;    // DRV8300DRGE_EVM
#else
#error Not select a right board for this project
#endif

#if defined(MOTOR1_ISBLDC) && (defined(MOTOR1_FAST) || \
    defined(MOTOR1_ESMO) || defined(MOTOR1_ENC) || defined(MOTOR1_HALL))
#error ISBLDC can't work with other estimaor simultaneously
#elif defined(MOTOR1_ENC) && defined(MOTOR1_HALL)
#error Can't support ENC and HALL simultaneously
#elif defined(MOTOR1_ESMO) && defined(MOTOR1_HALL)
#error Can't support ESMO and HALL simultaneously
#elif defined(MOTOR1_FAST) && defined(MOTOR1_ENC)
    systemVars.estType = EST_TYPE_FAST_ENC;     // the estimator is FAST and ENC
#elif defined(MOTOR1_FAST) && defined(MOTOR1_ESMO)
    systemVars.estType = EST_TYPE_FAST_ESMO;    // the estimator is FAST and ESMO
#elif defined(MOTOR1_FAST) && defined(MOTOR1_RESL)
    systemVars.estType = EST_TYPE_FAST_RESL;    // the estimator is FAST and RESOLVER
#elif defined(MOTOR1_FAST) && defined(MOTOR1_PSCOS)
    systemVars.estType = EST_TYPE_FAST_PSCOS;    // the estimator is FAST and SIN/COS Encoder
#elif defined(MOTOR1_FAST) && defined(MOTOR1_HALL)
    systemVars.estType = EST_TYPE_FAST_HALL;    // the estimator is FAST and HALL
#elif defined(MOTOR1_ESMO) && defined(MOTOR1_ENC)
    systemVars.estType = EST_TYPE_ESMO_ENC;     // the estimator is ESMO and ENC
#elif defined(MOTOR1_FAST)
    systemVars.estType = EST_TYPE_FAST;         // the estimator is only FAST
#elif defined(MOTOR1_ESMO)
    systemVars.estType = EST_TYPE_ESMO;         // the estimator is only ESMO
#elif defined(MOTOR1_ENC)
    systemVars.estType = EST_TYPE_ENC;          // the estimator is only ENC
#elif defined(MOTOR1_HALL)
    systemVars.estType = EST_TYPE_HALL;         // the estimator is only HALL
#elif defined(MOTOR1_ISBLDC)
    systemVars.estType = EST_TYPE_ISBLDC;       // the estimator is only ISBLDC
#else
#error Not select a right estimator for this project
#endif  // MOTOR1_FAST->MOTOR1_ENC

#if defined(MOTOR1_FAST)
    systemVars.estLibVersion = EST_getFASTVersion();   // gets FAST version
#endif  // MOTOR1_FAST


#if defined(MOTOR1_DCLINKSS) || defined(MOTOR1_ISBLDC)
    systemVars.currentSenseType = CURSEN_TYPE_SINGLE_SHUNT;
#elif defined(BSXL3PHGAN_REVA)
    systemVars.currentSenseType = CURSEN_TYPE_INLINE_SHUNT;
#else
    systemVars.currentSenseType = CURSEN_TYPE_THREE_SHUNT;
#endif  // Current Sense Type

#if defined(MOTOR1_HALL) && defined(_F280013x)
#error HALL sensors based FOC is not supported on this device
#endif  // MOTOR1_HALL & _F280013x

#if defined(MOTOR1_DCLINKSS) && defined(DRV8329AEVM_REVA)
// This kit supports single shunt
#elif defined(MOTOR1_DCLINKSS)
#error This kit doesn't support single shunt
// Only modificated BSXL8323RS_REVA and BSXL8323RS_REVA support single shunt
#endif  // MOTOR1_DCLINKSS

#if defined(MOTOR1_ISBLDC) && defined(DRV8329AEVM_REVA)
// This kit supports IS-BLDC
#elif defined(MOTOR1_ISBLDC)
#error This kit doesn't support InstaSPIN-BLDC
// Only modificated BSXL8323RS_REVA and BSXL8323RS_REVA support instaspin-bldc
#endif  // MOTOR1_ISBLDC

#if defined(DATALOGF2_EN) && defined(STEP_RP_EN)
#error DATALOG and GRAPH_STEP_RESPONSE can't be used simultaneously on this device
#endif  // DATALOGF2_EN && STEP_RP_EN

#if defined(MOTOR1_ISBLDC) && defined(MOTOR1_DCLINKSS)
#error Don't need to enable single shunt pre-define name if use instaspin-bldc
#endif  // MOTOR1_ISBLDC & MOTOR1_DCLINKSS

#if (defined(MOTOR1_SSIPD) || defined(MOTOR1_OVM)) && defined(MOTOR1_DCLINKSS)
#error Don't enable SSIPD and OVM if enable single shunt
#endif  // (MOTOR1_SSIPD | MOTOR1_OVM) & (MOTOR1_DCLINKSS

#if defined(MOTOR1_ISBLDC) && (defined(MOTOR1_OVM) || defined(MOTOR1_FWC) || \
        defined(MOTOR1_MTPA) || defined(MOTOR1_SSIPD))
#error Don't need to enable these functions if use instaspin-bldc
#endif  // MOTOR1_ISBLDC & (MOTOR1_OVM | MOTOR1_FWC | MOTOR1_MTPA | MOTOR1_SSIPD)

// ** above codes are only for checking the settings, not occupy the memory

    // Initialize device clock and peripherals
    Device_init();                  // call the function in device.c

    // Disable pin locks and enable internal pullups.
    Device_initGPIO();              // call the function in device.c

    // Initializes PIE and clears PIE registers. Disables CPU interrupts.
    Interrupt_initModule();         // call the function in driverlib.lib

    // Initializes the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    Interrupt_initVectorTable();    // call the function in driverlib.lib
   

  // EEPROM_WriteByte(1, 0xf0);// sample 0~ 1023//0xxx
  //EEPROM_WriteFloat(100, 3.14);
    // initialize the driver
    //eeprom_r=EEPROM_ReadFloat(100);
    halHandle = HAL_init(&hal, sizeof(hal));

    // set the driver parameters
    HAL_setParams(halHandle);

    // initialize the interrupt vector table
    HAL_initIntVectorTable(halHandle);

    // enable the ADC/PWM interrupts for control
    // enable interrupts to trig DMA
    HAL_enableCtrlInts(halHandle);

    // set the control parameters for motor 1

    motorHandle_M1 = (MOTOR_Handle)(&motorVars_M1);

    // set the reference speed, this can be replaced or removed
    motorVars_M1.flagEnableRunAndIdentify = false;

    motorVars_M1.speedRef_Hz = 60.0f;       // Hz
    motorVars_M1.speedRef_rpm = 600.0f;     // rpm

    // false - enables identification, true - disables identification
    userParams_M1.flag_bypassMotorId =  true;  //   true->false;   //syh

    initMotor1Handles(motorHandle_M1);
    initMotor1CtrlParameters(motorHandle_M1);

    // setup the GPIOs
    HAL_setupGPIOs(halHandle);

    // set up gate driver after completed GPIO configuration
    motorVars_M1.faultMtrNow.bit.gateDriver =
            HAL_MTR_setGateDriver(motorHandle_M1->halMtrHandle);

    // enable the ePWM module time base clock sync signal
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
    HAL_setupSPIB_Encoder(halHandle); 
    // Init_TCAN4550_ClassicCAN();
    //InitI2C(); //for eeprom
       // -----------------------------------------------------
    // 송신 헤더 설정 (Classic CAN 사양)


#if defined(CMD_POT_EN)
    setExtCmdPotParams(motorHandle_M1);
#endif  // CMD_POT_EN

#if defined(MOTOR1_HALL) && defined(CMD_CAP_EN)
#error HALL and CMD_CAP can't be enabled at the same time
#elif defined(CMD_CAP_EN)
    setExtCmdCapParams(motorHandle_M1);
#endif  // CMD_CAP_EN

#if defined(CMD_SWITCH_EN)
    setExtCmdSwitchParams(motorHandle_M1);
#endif  //CMD_SWITCH_EN

#ifdef CPUTIME_ENABLE
    // initialize the CPU usage module
    cpuTimeHandle = CPU_TIME_init(&cpuTime, sizeof(cpuTime));
    CPU_TIME_reset(cpuTimeHandle);
    CPU_TIME_setCtrlPeriod(cpuTimeHandle, HAL_getTimeBasePeriod(motorHandle_M1->halMtrHandle));
#endif  // CPUTIME_ENABLE


#if defined(EPWMDAC_MODE)
    // set DAC parameters
    pwmDACData.periodMax =
            PWMDAC_getPeriod(halHandle->pwmDACHandle[PWMDAC_NUMBER_1]);

    pwmDACData.ptrData[0] = &motorVars_M1.angleEST_rad;             // PWMDAC1
//    pwmDACData.ptrData[0] = &motorVars_M1.anglePLL_rad;             // PWMDAC1
//    pwmDACData.ptrData[1] = &motorVars_M1.angleENC_rad;             // PWMDAC1
//    pwmDACData.ptrData[1] = &motorVars_M1.angleHall_rad;            // PWMDAC1
//    pwmDACData.ptrData[1] = &motorVars_M1.angleGen_rad;             // PWMDAC2
//    pwmDACData.ptrData[1] = &motorVars_M1.adcData.I_A.value[0];     // PWMDAC2
    pwmDACData.ptrData[1] = &motorVars_M1.speedAbs_Hz;     // PWMDAC2
    pwmDACData.ptrData[1] = &motorVars_M1.speedAbs_Hz;     // PWMDAC3
//    pwmDACData.ptrData[2] = &motorVars_M1.adcData.I_A.value[1];     // PWMDAC3
    pwmDACData.ptrData[3] = &motorVars_M1.adcData.I_A.value[1];     // PWMDAC4

    pwmDACData.offset[0] = 0.5f;    // PWMDAC1
//    pwmDACData.offset[1] = 0.5f;    // PWMDAC2
    pwmDACData.offset[1] = 0.0f;    // PWMDAC2
    pwmDACData.offset[1] = 0.0f;    // PWMDAC3
//    pwmDACData.offset[2] = 0.5f;    // PWMDAC3
    pwmDACData.offset[3] = 0.5f;    // PWMDAC4

    pwmDACData.gain[0] = 1.0f / MATH_TWO_PI;                          // PWMDAC1
//    pwmDACData.gain[1] = 1.0f / MATH_TWO_PI;                        // PWMDAC2
//    pwmDACData.gain[1] = 1.0f / USER_M1_ADC_FULL_SCALE_CURRENT_A;   // PWMDAC2
    pwmDACData.gain[1] = 1.0f / USER_MOTOR1_FREQ_MAX_Hz;              // PWMDAC2
    pwmDACData.gain[2] = 1.0f / USER_MOTOR1_FREQ_MAX_Hz;              // PWMDAC3
//    pwmDACData.gain[2] = 1.0f / USER_M1_ADC_FULL_SCALE_CURRENT_A;   // PWMDAC3
    pwmDACData.gain[3] = 2.0f / USER_M1_ADC_FULL_SCALE_CURRENT_A;     // PWMDAC4
#endif  // EPWMDAC_MODE

#if defined(DATALOGF2_EN)
    // Initialize Datalog
    datalogHandle = DATALOGIF_init(&datalog, sizeof(datalog));
    DATALOG_Obj *datalogObj = (DATALOG_Obj *)datalogHandle;

    HAL_setupDMAforDLOG(halHandle, 0, &datalogBuff1[0], &datalogBuff1[1]);
    HAL_setupDMAforDLOG(halHandle, 1, &datalogBuff2[0], &datalogBuff2[1]);

#if (DMC_BUILDLEVEL <= DMC_LEVEL_2)
    // set datalog parameters
    datalogObj->iptr[0] = &motorVars_M1.adcData.I_A.value[0];
    datalogObj->iptr[1] = &motorVars_M1.adcData.I_A.value[1];
#elif (DMC_BUILDLEVEL == DMC_LEVEL_3)
    datalogObj->iptr[0] = &motorVars_M1.adcData.V_V.value[0];
    datalogObj->iptr[1] = &motorVars_M1.adcData.V_V.value[1];
#elif (DMC_BUILDLEVEL == DMC_LEVEL_4)
    datalogObj->iptr[0] = &motorVars_M1.angleFOC_rad;
    datalogObj->iptr[1] = &motorVars_M1.speed_Hz;
//    datalogObj->iptr[0] = &resl_M1.sin_os;
//    datalogObj->iptr[1] = &resl_M1.cos_os;
//    datalogObj->iptr[0] = &isbldc_M1.bemfInt;
//    datalogObj->iptr[1] = &isbldc_M1.VintPhase;
#endif  // DMC_BUILDLEVEL = DMC_LEVEL_1/2/3/4
#elif defined(DATALOGF4_EN) || defined(DATALOGI4_EN)
    // Initialize Datalog
    datalogHandle = DATALOGIF_init(&datalog, sizeof(datalog));
    DATALOG_Obj *datalogObj = (DATALOG_Obj *)datalogHandle;

    HAL_setupDMAforDLOG(halHandle, 0, &datalogBuff1[0], &datalogBuff1[1]);
    HAL_setupDMAforDLOG(halHandle, 1, &datalogBuff2[0], &datalogBuff2[1]);
    HAL_setupDMAforDLOG(halHandle, 2, &datalogBuff3[0], &datalogBuff3[1]);
    HAL_setupDMAforDLOG(halHandle, 3, &datalogBuff4[0], &datalogBuff4[1]);

#if (DMC_BUILDLEVEL <= DMC_LEVEL_2)
    // set datalog parameters
    datalogObj->iptr[0] = &motorVars_M1.adcData.I_A.value[0];
    datalogObj->iptr[1] = &motorVars_M1.adcData.I_A.value[1];
    datalogObj->iptr[2] = &motorVars_M1.adcData.I_A.value[2];
    datalogObj->iptr[3] = &motorVars_M1.angleFOC_rad;
#elif (DMC_BUILDLEVEL == DMC_LEVEL_3)
    datalogObj->iptr[0] = &motorVars_M1.adcData.V_V.value[0];
    datalogObj->iptr[1] = &motorVars_M1.adcData.V_V.value[1];
    datalogObj->iptr[2] = &motorVars_M1.adcData.V_V.value[2];
    datalogObj->iptr[3] = &motorVars_M1.angleFOC_rad;
#elif (DMC_BUILDLEVEL == DMC_LEVEL_4)
    datalogObj->iptr[0] = &motorVars_M1.angleFOC_rad;
    datalogObj->iptr[1] = &motorVars_M1.angleEST_rad;
    datalogObj->iptr[2] = &motorVars_M1.adcData.I_A.value[0];
    datalogObj->iptr[3] = &motorVars_M1.adcData.V_V.value[0];
#endif  // DMC_BUILDLEVEL = DMC_LEVEL_1/2/3/4
#endif  // DATALOGI4_EN


#if defined(DAC128S_ENABLE)
    // initialize the DAC128S
    dac128sHandle = DAC128S_init(&dac128s);


#if defined(BSXL8323RS_REVA) || defined(BSXL8353RS_REVA) || \
    defined(BSXL8316RT_REVA)
#if defined(_F280013x) || defined(_F280015x)    // DRV and DAC share SPIA
    // switch the SPI_STE pin for DRV device
    HAL_switchSPICS(motorHandle_M1->halMtrHandle);

    DEVICE_DELAY_US(1.0f);      // delay 1.0us

    // setup SPI for DAC128S
//    DAC128S_setupSPI(dac128sHandle);
    DAC128S_setupSPIBR(dac128sHandle, DACS_SPI_BITRATE);
#else   // !(_F280013x | F280015x)
    // setup SPI for DAC128S
      DEVICE_DELAY_US(1.0f);      // delay 1.0us

    // setup SPI for DAC128S
//    DAC128S_setupSPI(dac128sHandle);
    DAC128S_setupSPIBR(dac128sHandle, DACS_SPI_BITRATE);
    DAC128S_setupSPI(dac128sHandle);
    DAC128S_setupSPI(dac128sHandle);
#endif  // !(F280013x | F280015x)
#else   // !(BSXL8323RS_REVA | BSXL8353RS_REVA | BSXL8316RT_REVA)
    // setup SPI for DAC128S
    DAC128S_setupSPI(dac128sHandle);
#endif  // !(BSXL8323RS_REVA | BSXL8353RS_REVA | BSXL8316RT_REVA)



// The following settings are for output the values of different variables
// in each build level for debug. The User can select one of these groups in
// different build level as commented note

// DAC_LEVEL4_ISBLDC, DAC_LEVEL4_DCLINK, DAC_LEVEL4_VIBCOMP,
// DAC_LEVEL2_MOTOR1_VS, DAC_LEVEL2_MOTOR1_IS, DAC_LEVEL_MOTOR1_FAST,
// DAC_LEVEL4_FAST_ESMO, DAC_LEVEL4_FAST_ENC, DAC_LEVEL4_FAST_HALL
// DAC_LEVEL4_FAST, DAC_LEVEL4_FAST_ENC, DAC_LEVEL4_ENC, DAC_LEVEL4_HALL
// DAC_LEVEL4_PHADJ,

#if defined(MOTOR1_ISBLDC)
#define DAC_LEVEL4_ISBLDC               // define the DAC level
#elif defined(MOTOR1_FAST) && defined(MOTOR1_ESMO)
#define DAC_LEVEL4_FAST_ESMO            // define the DAC level
#else   // !MOTOR1_RESL && !MOTOR1_PSCOS && !MOTOR1_PSCOS
#define DAC_LEVEL_MOTOR1_FAST            // define the DAC level
#endif      // !MOTOR1_RESL && !MOTOR1_PSCOS

#if defined(DAC_LEVEL4_ISBLDC)
    dac128s.ptrData[0] = &isbldc_M1.VintPhase;              // CH_A
    dac128s.ptrData[1] = &isbldc_M1.bemfInt;                // CH_B
    dac128s.ptrData[2] = &isbldc_M1.Vabcn.value[0];         // CH_C
    dac128s.ptrData[3] = &isbldc_M1.Vabcn.value[1];         // CH_D

    dac128s.gain[0] = DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_VOLTAGE_V;
    dac128s.gain[1] = DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_VOLTAGE_V;
    dac128s.gain[2] = DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_VOLTAGE_V;
    dac128s.gain[3] = DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_VOLTAGE_V;

    dac128s.offset[0] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[1] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[2] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[3] = (uint16_t)(0.5f * DAC_SCALE_SET);
#elif defined(DAC_LEVEL4_DCLINK)
    // Build_Level_2, verify the current sampling value
    dac128s.ptrData[0] = &motorVars_M1.angleFOC_rad;                // CH_A
    dac128s.ptrData[1] = &motorVars_M1.adcData.I_A.value[0];        // CH_B
    dac128s.ptrData[2] = &motorVars_M1.adcIs_A.value[0];            // CH_C
    dac128s.ptrData[3] = &motorVars_M1.adcData.I_A.value[1];        // CH_D

    dac128s.gain[0] = DAC_SCALE_SET / MATH_TWO_PI;
    dac128s.gain[1] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;
    dac128s.gain[2] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;
    dac128s.gain[3] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;

    dac128s.offset[0] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[1] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[2] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[3] = (uint16_t)(0.5f * DAC_SCALE_SET);
#elif defined(DAC_LEVEL4_FAST_ESMO)
    dac128s.ptrData[0] = &motorVars_M1.angleEST_rad;                // CH_A
    dac128s.ptrData[1] = &motorVars_M1.anglePLL_rad;                // CH_B
#if defined(ESMO_DEBUG)
    dac128s.ptrData[2] = &esmo_M1.thetaElec_rad;                    // CH_C
#else   //!ESMO_DEBUG
    dac128s.ptrData[2] = &motorVars_M1.adcData.I_A.value[0];        // CH_C
#endif  //!ESMO_DEBUG
    dac128s.ptrData[3] = &motorVars_M1.adcData.I_A.value[1];        // CH_D

    dac128s.gain[0] = DAC_SCALE_SET / MATH_TWO_PI;
    dac128s.gain[1] = DAC_SCALE_SET / MATH_TWO_PI;
#if defined(ESMO_DEBUG)
    dac128s.gain[2] = DAC_SCALE_SET / MATH_TWO_PI;
#else   //!ESMO_DEBUG
    dac128s.gain[2] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;
#endif  //!ESMO_DEBUG
    dac128s.gain[3] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;

    dac128s.offset[0] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[1] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[2] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[3] = (uint16_t)(0.5f * DAC_SCALE_SET);
#elif defined(DAC_LEVEL_MOTOR1_FAST)
    dac128s.ptrData[0] = &motorVars_M1.angleFOC_rad;                // CH_A
    dac128s.ptrData[1] = &motorVars_M1.adcData.I_A.value[0];        // CH_B
    dac128s.ptrData[2] = &motorVars_M1.adcData.I_A.value[1];        // CH_C
    dac128s.ptrData[3] = &motorVars_M1.adcData.I_A.value[2];        // CH_D

    dac128s.gain[0] = DAC_SCALE_SET / MATH_TWO_PI;
    dac128s.gain[1] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;
    dac128s.gain[2] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;
    dac128s.gain[3] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;

    dac128s.offset[0] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[1] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[2] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[3] = (uint16_t)(0.5f * DAC_SCALE_SET);
#elif defined(DAC_LEVEL4_FAST_ESMO)
    dac128s.ptrData[0] = &motorVars_M1.angleEST_rad;                // CH_B
    dac128s.ptrData[1] = &motorVars_M1.anglePLL_rad;                // CH_B
    dac128s.ptrData[2] = &motorVars_M1.adcData.I_A.value[0];        // CH_D
    dac128s.ptrData[3] = &motorVars_M1.adcData.I_A.value[1];        // CH_E

    dac128s.gain[0] = DAC_SCALE_SET / MATH_TWO_PI;
    dac128s.gain[1] = DAC_SCALE_SET / MATH_TWO_PI;
    dac128s.gain[2] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;
    dac128s.gain[3] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;

    dac128s.offset[0] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[1] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[2] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[3] = (uint16_t)(0.5f * DAC_SCALE_SET);
#elif defined(DAC_LEVEL4_FAST_ENC)
    dac128s.ptrData[0] = &motorVars_M1.angleEST_rad;                // CH_A
    dac128s.ptrData[1] = &motorVars_M1.angleENC_rad;                // CH_B
    dac128s.ptrData[2] = &motorVars_M1.adcData.I_A.value[0];        // CH_C
    dac128s.ptrData[3] = &motorVars_M1.adcData.I_A.value[1];        // CH_D

    dac128s.gain[0] = DAC_SCALE_SET / MATH_TWO_PI;
    dac128s.gain[1] = DAC_SCALE_SET / MATH_TWO_PI;
    dac128s.gain[2] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;
    dac128s.gain[3] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;

    dac128s.offset[0] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[1] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[2] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[3] = (uint16_t)(0.5f * DAC_SCALE_SET);
#elif defined(DAC_LEVEL4_FAST_HALL)
    dac128s.ptrData[0] = &motorVars_M1.angleEST_rad;                // CH_A
    dac128s.ptrData[1] = &motorVars_M1.angleHall_rad;                // CH_B
    dac128s.ptrData[2] = &motorVars_M1.adcData.I_A.value[0];        // CH_C
    dac128s.ptrData[3] = &motorVars_M1.adcData.I_A.value[1];        // CH_D

    dac128s.gain[0] = DAC_SCALE_SET / MATH_TWO_PI;
    dac128s.gain[1] = DAC_SCALE_SET / MATH_TWO_PI;
    dac128s.gain[2] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;
    dac128s.gain[3] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;

    dac128s.offset[0] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[1] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[2] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[3] = (uint16_t)(0.5f * DAC_SCALE_SET);
#elif defined(DAC_LEVEL4_FAST)
    dac128s.ptrData[0] = &motorVars_M1.angleFOC_rad;                // CH_A
    dac128s.ptrData[1] = &motorVars_M1.adcData.I_A.value[0];        // CH_B
    dac128s.ptrData[2] = &motorVars_M1.adcData.V_V.value[0];        // CH_C
    dac128s.ptrData[3] = &motorVars_M1.adcData.I_A.value[1];        // CH_D

    dac128s.gain[0] = DAC_SCALE_SET / MATH_TWO_PI;
    dac128s.gain[1] = DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;
    dac128s.gain[2] = DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_VOLTAGE_V;
    dac128s.gain[3] = DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;

    dac128s.offset[0] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[1] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[2] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[3] = (uint16_t)(0.5f * DAC_SCALE_SET);
#elif defined(DAC_LEVEL4_PHADJ)
    dac128s.ptrData[0] = &motorVars_M1.Vab_out_V.value[0];          // CH_A
    dac128s.ptrData[1] = &motorVars_M1.estInputData.Iab_A.value[0]; // CH_B
    dac128s.ptrData[2] = &motorVars_M1.Eab_V.value[0];              // CH_C
    dac128s.ptrData[3] = &motorVars_M1.Eab_V.value[1];             // CH_D

    dac128s.gain[0] = DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_VOLTAGE_V;
    dac128s.gain[1] = DAC_SCALE_SET * 4.0f / USER_M1_ADC_FULL_SCALE_CURRENT_A;
    dac128s.gain[2] = DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_VOLTAGE_V;
    dac128s.gain[3] = DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_VOLTAGE_V;

    dac128s.offset[0] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[1] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[2] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[3] = (uint16_t)(0.5f * DAC_SCALE_SET);
#elif defined(DAC_LEVEL3_MOTOR1_FAST)
    // Build_Level_2 or Level_3, verify the estimator
    dac128s.ptrData[0] = &motorVars_M1.angleGen_rad;                // CH_A
    dac128s.ptrData[1] = &motorVars_M1.angleEST_rad;                // CH_B
    dac128s.ptrData[2] = &motorVars_M1.adcData.I_A.value[0];        // CH_C
    dac128s.ptrData[3] = &motorVars_M1.adcData.I_A.value[1];        // CH_D

    dac128s.gain[0] = DAC_SCALE_SET / MATH_TWO_PI;
    dac128s.gain[1] = DAC_SCALE_SET / MATH_TWO_PI;
    dac128s.gain[2] = DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;
    dac128s.gain[3] = DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;

    dac128s.offset[0] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[1] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[2] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[3] = (uint16_t)(0.5f * DAC_SCALE_SET);
#elif defined(DAC_LEVEL2_MOTOR1_IS)
    // Build_Level_2 or Level_3, verify the estimator
    dac128s.ptrData[0] = &motorVars_M1.angleEST_rad;                // CH_A
    dac128s.ptrData[1] = &motorVars_M1.adcData.I_A.value[0];        // CH_B
    dac128s.ptrData[2] = &motorVars_M1.adcData.I_A.value[1];        // CH_C
    dac128s.ptrData[3] = &motorVars_M1.adcData.I_A.value[2];        // CH_D

    dac128s.gain[0] = DAC_SCALE_SET / MATH_TWO_PI;
    dac128s.gain[1] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;
    dac128s.gain[2] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;
    dac128s.gain[3] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_CURRENT_A;

    dac128s.offset[0] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[1] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[2] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[3] = (uint16_t)(0.5f * DAC_SCALE_SET);
#elif defined(DAC_LEVEL2_MOTOR1_VS)
    // Build_Level_2 or Level_3, verify the estimator
    dac128s.ptrData[0] = &motorVars_M1.angleEST_rad;                // CH_A
    dac128s.ptrData[1] = &motorVars_M1.adcData.V_V.value[0];        // CH_B
    dac128s.ptrData[2] = &motorVars_M1.adcData.V_V.value[1];        // CH_C
    dac128s.ptrData[3] = &motorVars_M1.adcData.V_V.value[2];        // CH_D

    dac128s.gain[0] = DAC_SCALE_SET / MATH_TWO_PI;
    dac128s.gain[1] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_VOLTAGE_V;
    dac128s.gain[2] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_VOLTAGE_V;
    dac128s.gain[3] = 2.0f * DAC_SCALE_SET / USER_M1_ADC_FULL_SCALE_VOLTAGE_V;

    dac128s.offset[0] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[1] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[2] = (uint16_t)(0.5f * DAC_SCALE_SET);
    dac128s.offset[3] = (uint16_t)(0.5f * DAC_SCALE_SET);
#endif  // (DMC_BUILDLEVEL <= DMC_LEVEL_3)


  //  DAC128S_writeCommand(dac128sHandle); new board
#endif  // DAC128S_ENABLE


#if defined(SFRA_ENABLE)
    // Plot GH & H plots using SFRA_GUI, GH & CL plots using SFRA_GUI_MC
    configureSFRA(SFRA_GUI_PLOT_GH_H, USER_M1_ISR_FREQ_Hz);

    sfraNoiseId = 0.0f;
    sfraNoiseIq = 0.0f;
    sfraNoiseSpd = 0.0f;
    sfraNoiseOut = 0.0f;
    sfraNoiseFdb = 0.0f;
    sfraTestLoop = SFRA_TEST_D_AXIS;
    sfraCollectStart = false;
   // configureSFRA();


#endif  // SFRA_ENABLE

#if defined(STEP_RP_EN)
    GRAPH_init(&stepRPVars,
               &motorVars_M1.speedRef_Hz, &motorVars_M1.speed_Hz,
               &motorVars_M1.IdqRef_A.value[0], &motorVars_M1.Idq_in_A.value[0],
               &motorVars_M1.IdqRef_A.value[1], &motorVars_M1.Idq_in_A.value[1]);
#endif  // STEP_RP_EN

    systemVars.flagEnableSystem = true;

#if defined(CMD_CAN_EN)
    // initialize the CANCOM
    initCANCOM(halHandle);

    motorVars_M1.cmdCAN.speedSet_Hz = 40.0f;

    motorVars_M1.cmdCAN.flagEnableCmd = false;
    motorVars_M1.cmdCAN.flagEnableSyncLead = false;
#endif // CMD_CAN_EN

    motorVars_M1.flagEnableOffsetCalc = true;

    // run offset calibration for motor 1
    runMotor1OffsetsCalculation(motorHandle_M1);

#if defined(MOTOR1_RESL)
    motorVars_M1.flagEnableRESLAdcOffsetCalc = true;

    // run offset calibration for resolver 1
    runResolver1OffsetsCalculation(motorHandle_M1);
#endif  // MOTOR1_RESL

    memcpy((uint32_t *)&Cla1ProgRunStart, (uint32_t *)&Cla1ProgLoadStart, (uint32_t)&Cla1ProgLoadSize);
    HAL_setupCLA(halHandle); //cladisable syh.
    // enable global interrupts
    HAL_enableGlobalInts(halHandle);

    // enable debug interrupts
    HAL_enableDebugInt(halHandle);
    InitI2C();
   // HAL_setupSPIB_Encoder();
    /*test code*/
    //  dac128sHandle = DAC128S_init(&dac128s);
   // HAL_switchSPICS(motorHandle_M1->halMtrHandle);
         DEVICE_DELAY_US(1.0f);      // delay 1.0us
// ==============================================================
    // [추가] 부팅 시 EEPROM에서 주요 파라미터 로드
    // ==============================================================
    float tempParam = 0.0f;

    // 1) Kp 파라미터 (Index 1) 로드
    tempParam = EEPROM_readFloatByIndex(1);
    // 읽어온 값이 비정상적인 쓰레기값(0xFFFFFFFF 등)이 아닐 경우에만 적용
    if(tempParam > -10000.0f && tempParam < 10000.0f) {
        motorVars_M1.pos_Kp = tempParam;
    } else {
        motorVars_M1.pos_Kp = 5.0f; // 데이터가 깨졌거나 비어있을 경우 기본값 강제 할당
    }

    // 2) Ki 파라미터 (Index 2) 로드
    tempParam = EEPROM_readFloatByIndex(2);
    if(tempParam > -10000.0f && tempParam < 10000.0f) {
        motorVars_M1.pos_Ki = tempParam;
    } else {
        motorVars_M1.pos_Ki = 0.0f; // 기본값
    }

    // 3) Preset1 파라미터 (Index 3) 로드
    tempParam = EEPROM_readFloatByIndex(3);
    if(tempParam > -100000.0f && tempParam < 100000.0f) {
        preset1 = tempParam;
    } else {
        preset1 = 0.0f; // 기본값
    }

    // 4) Preset2 파라미터 (Index 4) 로드
    tempParam = EEPROM_readFloatByIndex(4);
    if(tempParam > -100000.0f && tempParam < 100000.0f) {
        preset2 = tempParam;
    } else {
        preset2 = 0.0f; // 기본값
    }

    // 5) Preset3 파라미터 (Index 5) 로드
    tempParam = EEPROM_readFloatByIndex(5);
    if(tempParam > -100000.0f && tempParam < 100000.0f) {
        preset3 = tempParam;
    } else {
        preset3 = 0.0f; // 기본값
    }
   // ==============================================================
    // 6) Current_limit 파라미터 (Index 6) 로드
    // ==============================================================
    tempParam = EEPROM_readFloatByIndex(6);
    // 전류값이 정상 범위(예: 0.1A ~ 100.0A) 내에 있는지 방어 로직
    if(tempParam >= 0.0f && tempParam <= 100.0f) {
        motorVars_M1.Current_limit = tempParam;
    } else {
        motorVars_M1.Current_limit = 1.0f; // 데이터가 깨졌을 경우 초기 기본값(1.0f)
    }

    // ==============================================================
    // 7) Torque_limit 파라미터 (Index 7) 로드
    // ==============================================================
    tempParam = EEPROM_readFloatByIndex(7);
    // 토크 제한값이 정상 범위 내에 있는지 방어 로직
    if(tempParam >= 0.0f && tempParam <= 100.0f) {
        motorVars_M1.Torque_limit = tempParam;
    } else {
        motorVars_M1.Torque_limit = 1.0f; // 데이터가 깨졌을 경우 초기 기본값(1.0f)
    }
    // ==============================================================
    // 8) pos_kp2 파라미터 (Index 8) 로드
    // ==============================================================
    tempParam = EEPROM_readFloatByIndex(8);
    // 값이 0이 아니며, 비정상적인 쓰레기값이 아닐 때만 적용
    if(tempParam > -10000.0f && tempParam < 10000.0f && tempParam != 0.0f) {
        poskp_2 = tempParam;
    } else {
        poskp_2 = 10.0f; // 0이거나 깨졌을 경우 기본값 10.0f
    }
    // ==============================================================
    // 9) dead_zone 파라미터 (Index 9) 로드
    // ==============================================================
    tempParam = EEPROM_readFloatByIndex(9);
    // 값이 0이 아닐 때만 적용 (정상 범위 조건 포함)
    if(tempParam > -100.0f && tempParam < 100.0f && tempParam != 0.0f) {
        motorVars_M1.dead_zone = tempParam;
    } else {
        motorVars_M1.dead_zone = 0.2f; // 0이거나 깨졌을 경우 기본값 0.2
    }

    // ==============================================================
    // 10) dead_zone_hys 파라미터 (Index 10) 로드
    // ==============================================================
    tempParam = EEPROM_readFloatByIndex(10);
    // 값이 0이 아닐 때만 적용 (정상 범위 조건 포함)
    if(tempParam > -100.0f && tempParam < 100.0f && tempParam != 0.0f) {
        motorVars_M1.dead_zone_hys = tempParam;
    } else {
        motorVars_M1.dead_zone_hys = 0.1f; // 0이거나 깨졌을 경우 기본값 0.1
    }
    // ==============================================================



    // setup SPI for DAC128S
//    DAC128S_setupSPI(dac128sHandle);
   // DAC128S_setupSPIBR(dac128sHandle, DACS_SPI_BITRATE);
 //   DAC128S_setupSPI(dac128sHandle);
  //   DAC128S_setupSPI(dac128sHandle);
    
 
      // EEPROM_WriteByte(1, 0xf0);// sample 0~ 1023//0xxx
  //EEPROM_WriteFloat(100, 3.14);
    // initialize the driver
    //eeprom_r=EEPROM_ReadFloat(100);

    systemVars.powerRelayWaitTime_ms = POWER_RELAY_WAIT_TIME_ms;
   for(ci=0; ci<8; ci++)
    {
    motorVars_M1.bitArray1[ci] = 0;
    motorVars_M1.bitArray2[ci] = 0;
}
        /*test code by kang*/
       

    // Waiting for enable system flag to be set
    while(systemVars.flagEnableSystem == false)
    {
        if(HAL_getCPUTimerStatus(halHandle, HAL_CPU_TIMER0))
        {
            HAL_clearCPUTimerFlag(halHandle, HAL_CPU_TIMER0);

            systemVars.timerBase_1ms++;

            if(systemVars.timerBase_1ms > systemVars.powerRelayWaitTime_ms)
            {
                systemVars.flagEnableSystem = true;
                systemVars.timerBase_1ms = 0;
            }
        }
    }

    motorVars_M1.flagInitializeDone = true;


    GPIO_writePin(12, 0); 
    GPIO_writePin(26, 1); 
    SPI_InitSequence();

    while(systemVars.flagEnableSystem == true)
    {
        // loop while the enable system flag is true
        systemVars.mainLoopCnt++;
        IG_IN=GPIO_readPin(25);
        if(IG_OUT==1) 
        GPIO_writePin(16, 1);
        else
        GPIO_writePin(16, 0);
        
        /*test code by kang*/
          // -----------------------------------------------------
        // 1. CAN 메시지 수신 (RX FIFO 0 확인)
        // -----------------------------------------------------
       // uint8_t messagesReceived = TCAN4x5x_MCAN_ReadNextFIFO(RXFIFO0, &rxHeader, rxData);
      //  if (messagesReceived > 0)
      //  {
            // 수신 데이터(rxData)를 모터 제어 변수 등에 반영
            // 예: if(rxHeader.ID == 0x100) { motorVars_M1.speedRef_Hz = rxData[0]; }
     //   }

        // -----------------------------------------------------
        // 2. CAN 메시지 송신 (특정 타이머 혹은 이벤트 발생 시)
        // -----------------------------------------------------
        static uint16_t txTimer = 0;
        if (++txTimer > 10000) // 송신 주기 예시
        {
            txTimer = 0;
            
            // 전송할 8바이트 데이터 채우기
            txData[0] = 0x65;
            txData[1] = 0x00;
            txData[2] = 0x00;
            txData[3] = 0x01;
            txData[0] = 0x00;
            txData[0] = 0x00;
            txData[0] = 0x00;
            txData[0] = 0x00;
            
            // ... (모터 상태 데이터 등)
            
            // TX 버퍼 인덱스 0번에 메시지 쓰기
         //   TCAN4x5x_MCAN_WriteTXBuffer(0, &txHeader, txData);
            
            // 버퍼 0번 전송 명령
          //  TCAN4x5x_MCAN_TransmitBufferContents(0);
        }
    

        if(reset==1)
        {
            GPIO_writePin(12, 1); // CS HIGH 종료

        }
         

        // 1ms time base
        if(HAL_getCPUTimerStatus(halHandle, HAL_CPU_TIMER0))
        {
            HAL_clearCPUTimerFlag(halHandle, HAL_CPU_TIMER0);

            // toggle status LED on controller board
            systemVars.counterLEDC++;

            if(systemVars.counterLEDC > (uint16_t)(LED_BLINK_FREQ_Hz * 1000))
            {
#if defined(_F28P65x)
                HAL_toggleGPIO(halHandle, HAL_GPIO_LED2C);     // Toggle on the LED
#else
                HAL_toggleGPIO(halHandle, HAL_GPIO_LED1C);     // Toggle on the LED
#endif
              
        //   I2C_sendStopCondition(I2CB_BASE);// test code 
      //     I2C_sendStartCondition(I2CB_BASE);// test code 
           //  DEVICE_DELAY_US(5000);  test code
             // eeprom_r=EEPROM_readByte(1);
             if(eep_read==1)
             {
ab = EEPROM_readFloatByIndex(addr+4);
//cd= EEPROM_readFloatByIndex(9);
eeprom_r=EEPROM_readByte(addr);
             }
             if(eep_write==1)
              {
                EEPROM_writeFloatByIndex(addr+4, cd);
               // EEPROM_writeFloatByIndex(9, 4.56f);
               EEPROM_writeByte(addr,temp_data);

              }
//DEVICE_DELAY_US(5000);
            //  I2C_sendStopCondition(I2CB_BASE);// test code
//DEVICE_DELAY_US(5000);
           //  wc_s=EEPROM_WriteByte(1,101);
              //  DEVICE_DELAY_US(5000);
            //    I2C_sendStopCondition(I2CB_BASE);// test code
           //  DEVICE_DELAY_US(5000);
           // EEPROM_writeByte(1,99);
            // DEVICE_DELAY_US(5000);
            //  I2C_sendStopCondition(I2CB_BASE);// test code
             // DEVICE_DELAY_US(5000);
         //   EEPROM_writeByte(1,98);
//DEVICE_DELAY_US(5000); test code
            //  I2C_sendStopCondition(I2CB_BASE);// test code
         //   DEVICE_DELAY_US(5000);
           //  wc_s=EEPROM_WriteByte(3,102);
              //  DEVICE_DELAY_US(5000);
                //eeprom_r=EEPROM_ReadFloat(100);
                
               systemVars.counterLEDC = 0;
               
            }

            if(motorVars_M1.motorState >= MOTOR_CL_RUNNING)
            {
                systemVars.timeWaitLEDB =
                        (uint16_t)(40000.0f / (fabsf(motorVars_M1.speed_Hz) + 20.0f));

                // toggle status LED on inverter board if have
                systemVars.counterLEDB++;

                if(systemVars.counterLEDB > systemVars.timeWaitLEDB)
                {
                    HAL_toggleGPIO(halHandle, HAL_GPIO_LED1B);     // Toggle on the LED

                    systemVars.counterLEDB = 0;
                }
            }
            else
            {
                HAL_setGPIOHigh(halHandle, HAL_GPIO_LED1B);     // Turn on the LED
            }

            systemVars.timerBase_1ms++;
            enc_timer++;

            switch(systemVars.timerBase_1ms)
            {
                case 1:     // motor 1 protection check
                    runMotorMonitor(motorHandle_M1);
                    break;
                case 2:
                    calculateRMSData(motorHandle_M1);
                    break;
                case 3:
#if defined(MOTOR1_PI_TUNE)
                    // Tune the gains of the controllers
                    tuneControllerGains(motorHandle_M1);
#endif      // MOTOR1_PI_TUNE
                    break;
                case 4:     // calculate motor protection value
                    calcMotorOverCurrentThreshold(motorHandle_M1);
                    break;
                case 5:     // system control
                    systemVars.timerBase_1ms = 0;
                    systemVars.timerCnt_5ms++;
                    break;
            }

#if defined(CMD_CAN_EN)
            //MOTOR_Vars_t *objMtr = Motor_getVars(handle);
           
           //can a
           updateCANCmdFreq(motorHandle_M1);//syh
           updateCANSetupParams(); // new
           
           static uint8_t step = 0;
  //can b
 #if 0
        if(step == 0)
            B_CAN_Interrupt_Check();
    
         else if(step == 1)
             {
        if(rx[3] != 0 && rx[3] != 0xFFFF)
                {
               SPI_TX_build2(0, 0x8408, 2, rx_data);
                SPI_OnceTransaction2();
                DINT;
                CAN_B_DATA_READ(motorHandle_M1);
                EINT;
                clear_flag=1;
               }
             }
    else if(step == 2 && clear_flag==1)
    {
        SPI_TX_build(1, 0x1050, 1, 0x00000005);
        SPI_OnceTransaction();
        clear_flag=0;
        tx_build_flag=1;
    }
    else if(step == 3 && tx_build_flag==1)
    {
        tx_flag1=1;
        tx_build_flag=0;
       
        SPI_TX_build2(1, 0x8008, 2, tx_data);
        SPI_OnceTransaction2();
    }
    else if(step == 4 && tx_flag1==1)
    {
        tx_flag1=0;
        SPI_TX_build(1, 0x10D0, 1, 0x00000001);
        SPI_OnceTransaction();
    }
    step++;

    if(step > 4)
        step = 0;
  #endif

           
           //SFRA_GUI_runSerialHostComms(&sfra1);// syh
           if(enc_timer==2000)
           {
   float adcValue = (float)motorVars_M1.adcData.userB[1];
float pt_Resistance = 0.0f;

// 1. 단선/단락(Short/Open) 등 하드웨어 예외 처리
if(adcValue <= 1.0f) 
{
    motorTemp = 999.0f;
}
else if(adcValue >= 4095.0f) 
{
    motorTemp = -999.0f;
}
else 
{
    // 2. ADC 값을 PT1000 저항값으로 1회만 계산 (연산량 대폭 감소)
    // 회로 구조: 5V -> PT1000 -> ADC 핀 -> 1.5k옴(1500) 풀다운 -> GND
    pt_Resistance = 1500.0f * ((4095.0f * 5.0f) / (adcValue * 3.3f) - 1.0f);

    // 3. 온도 변환
    if(pt_Resistance >= 1000.0f) 
    {
        // 영상 온도 (0도 이상): Callendar-Van Dusen 2차 방정식 적용
        motorTemp = (-3.9083e-3f + sqrtf((3.9083e-3f * 3.9083e-3f) 
                    - 4.0f * (-5.775e-7f) * (1.0f - (pt_Resistance / 1000.0f)))) 
                    / (2.0f * (-5.775e-7f));
    }
    else 
    {
        // 영하 온도 (0도 미만): PT1000 표준 선형 근사식 적용
        // PT1000은 영하 구간에서 선형성이 매우 좋아 이 공식으로 -50도까지 오차 없이 안정적으로 계산됩니다.
        motorTemp = (pt_Resistance - 1000.0f) / 3.85f;
    }
}
           boardTemp=(motorVars_M1.adcData.userB[2]/4096)*218.75-66.875;
           boardHumi=(motorVars_M1.adcData.userB[3]/4096)*125-12.5;
           //motorVars_M1.position_enc=ReadEncoderSSI();  //-> motor isr // new board test
           enc_timer=0;
          
           }

            if((motorVars_M1.cmdCAN.flagEnableCmd == true) && (motorVars_M1.faultMtrUse.all == 0))
            {
                canComVars.flagCmdTxRun = motorVars_M1.cmdCAN.flagCmdRun;
                canComVars.speedSet_Hz = motorVars_M1.cmdCAN.speedSet_Hz;

                if(motorVars_M1.cmdCAN.flagEnableSyncLead == true)
                {
                    motorVars_M1.flagEnableRunAndIdentify = motorVars_M1.cmdCAN.flagCmdRun;
                    motorVars_M1.speedRef_Hz = motorVars_M1.cmdCAN.speedSet_Hz;
                }
                else
                {
                    motorVars_M1.flagEnableRunAndIdentify = canComVars.flagCmdRxRun;
                    motorVars_M1.speedRef_Hz = canComVars.speedRef_Hz;
                }
            }
#endif // CMD_CAN_EN

#if defined(CMD_POT_EN)
            updateExtCmdPotFreq(motorHandle_M1);
#endif  // CMD_POT_EN

#if defined(MOTOR1_HALL) && defined(CMD_CAP_EN)
#error HALL and CMD_CAP can't be enabled at the same time
#elif defined(CMD_CAP_EN)
            updateExtCmdCapFreq(motorHandle_M1,
                                HAL_calcCAPCount(motorHandle_M1->halMtrHandle));
#endif  // CMD_CAP_EN

#if defined(CMD_SWITCH_EN)
            updateCmdSwitch(motorHandle_M1);
#endif  //CMD_SWITCH_EN

#if defined(SFRA_ENABLE)
            // SFRA test
            SFRA_F32_runBackgroundTask(&sfra1);
            SFRA_GUI_runSerialHostComms(&sfra1);
#endif  // SFRA_ENABLE

#if defined(STEP_RP_EN)
            // Generate Step response
            GRAPH_generateStepResponse(&stepRPVars);
#endif  // STEP_RP_EN

#ifdef CPUTIME_ENABLE
            CPU_TIME_calcCPUWidthRatio(cpuTimeHandle);
#endif  // CPUTIME_ENABLE

        }       // 1ms Timer

if(g_flagEepromSaveReq == true)
        {
            EEPROM_writeFloatByIndex(g_eepromSaveIndex, g_eepromSaveValue);
            g_flagEepromSaveReq = false;
        }

        // [기능 구현] 전체 파라미터 일괄 저장 (Index 0 명령 수신 시)
        if(g_flagEepromSaveAllReq == true)
        {
            EEPROM_writeFloatByIndex(1, motorVars_M1.pos_Kp);
            EEPROM_writeFloatByIndex(2, motorVars_M1.pos_Ki);
            EEPROM_writeFloatByIndex(3, preset1);
            EEPROM_writeFloatByIndex(4, preset2);
            EEPROM_writeFloatByIndex(5, preset3);
            EEPROM_writeFloatByIndex(6, motorVars_M1.Current_limit); 
            EEPROM_writeFloatByIndex(7, motorVars_M1.Torque_limit);  
            EEPROM_writeFloatByIndex(8, poskp_2);                    // [수정] 전역 변수 pos_kp2 저장
            EEPROM_writeFloatByIndex(9, motorVars_M1.dead_zone);     
            EEPROM_writeFloatByIndex(10, motorVars_M1.dead_zone_hys);
            g_flagEepromSaveAllReq = false;
        }




#if defined(CMD_SWITCH_EN)
        outputCmdState(motorHandle_M1);
#endif  //CMD_SWITCH_EN

        // runs control for motor 1
        runMotor1Control(motorHandle_M1);

        // Read/Write the registers of DRV device
#if defined(BSXL8323RS_REVA) || defined(BSXL8353RS_REVA) || \
    defined(BSXL8316RT_REVA)
#if defined(_F280013x) || defined(_F280015x)    // DRV and DAC share SPIA
#if defined(DAC128S_ENABLE)
        if(HAL_getDRVFlagWR(motorHandle_M1->halMtrHandle) == true)
        {
            if(HAL_getSelectionSPICS(motorHandle_M1->halMtrHandle) != SPI_CS_DRV)
            {
                // switch the SPI_STE pin for DRV device
                HAL_switchSPICS(motorHandle_M1->halMtrHandle);

                DEVICE_DELAY_US(1.0f);      // delay 1.0us

                // setup the spi for drv8323/drv8353/drv8316
                HAL_setupSPI(motorHandle_M1->halMtrHandle);

                DEVICE_DELAY_US(1.0f);      // delay 1.0us
            }

            HAL_writeDRVData(motorHandle_M1->halMtrHandle, &drvicVars_M1);
            HAL_readDRVData(motorHandle_M1->halMtrHandle, &drvicVars_M1);
        }
        else if(HAL_getSelectionSPICS(motorHandle_M1->halMtrHandle) != SPI_CS_DAC)
        {
            // switch the SPI_STE pin for DRV device
            HAL_switchSPICS(motorHandle_M1->halMtrHandle);

            DEVICE_DELAY_US(1.0f);      // delay 1.0us

            // setup SPI for DAC128S
            DAC128S_setupSPIBR(dac128sHandle, DACS_SPI_BITRATE);

            DEVICE_DELAY_US(1.0f);      // delay 1.0us
        }
#else  // !DAC128S_ENABLE
        HAL_writeDRVData(motorHandle_M1->halMtrHandle, &drvicVars_M1);
        HAL_readDRVData(motorHandle_M1->halMtrHandle, &drvicVars_M1);
#endif  // !DAC128S_ENABLE
#elif defined(_F28002x) || defined(_F28003x) || defined(_F28P65x)
        HAL_writeDRVData(motorHandle_M1->halMtrHandle, &drvicVars_M1); // drv write
        HAL_readDRVData(motorHandle_M1->halMtrHandle, &drvicVars_M1);



   

    
#else
#error This lab doesn't support thses devices, you need to change some files
#endif  // !(F280013x | F280015x | F28002x | F28003x)
#endif  // BSXL8323RS_REVA | BSXL8353RS_REVA | BSXL8316RT_REVA

    } // end of while() loop

    // disable the PWM
    HAL_disablePWM(motorHandle_M1->halMtrHandle);

} // end of main() function

//
//-- end of this file ----------------------------------------------------------
//
