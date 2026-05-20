#ifndef TCAN4X5X_SPI_H_
#define TCAN4X5X_SPI_H_

// C2000 디바이스 및 드라이버 라이브러리 포함
#include "driverlib.h"
#include "device.h"

// F280039C 하드웨어에 맞게 SPI Base 및 CS 핀 정의
// (실제 연결된 SPI 채널과 GPIO 핀 번호로 변경하세요. 예: SPI A채널, GPIO 19)
#define SPI_HW_ADDR       SPIA_BASE
#define SPI_CS_GPIO_PIN   27

// AHB Access Op Codes
#define AHB_WRITE_OPCODE 0x61
#define AHB_READ_OPCODE  0x41

// Write Functions
void AHB_WRITE_32(uint16_t address, uint32_t data);
void AHB_WRITE_BURST_START(uint16_t address, uint8_t words);
void AHB_WRITE_BURST_WRITE(uint32_t data);
void AHB_WRITE_BURST_END(void);

// Read Functions
uint32_t AHB_READ_32(uint16_t address);
void AHB_READ_BURST_START(uint16_t address, uint8_t words);
uint32_t AHB_READ_BURST_READ(void);
void AHB_READ_BURST_END(void);

#endif /* TCAN4X5X_SPI_H_ */