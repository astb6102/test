#include "TCAN4x5x_SPI.h"

/* 
 * 내부 헬퍼 함수: C2000에서 8비트 SPI 송수신 처리
 * (SPI 모듈이 8비트 데이터 길이로 초기화되었다고 가정합니다.)
 * C2000은 왼쪽 정렬(<<8)하여 송신 버퍼에 넣어야 정상적으로 8비트가 전송됩니다.


 */

 static inline uint16_t SPI_transferByte(uint32_t base, uint16_t txData) {
    SPI_writeDataBlockingFIFO(base, txData & 0xFF);
    return SPI_readDataBlockingFIFO(base); // 송신한 만큼 정확히 읽어내어 RX 버퍼를 비움
}

static inline uint8_t SPI_transmit8Bit(uint16_t data)
{
    SPI_writeDataBlockingNonFIFO(SPI_HW_ADDR, data << 8);
    // 송신 후 수신 버퍼에서 데이터를 읽고 하위 8비트만 마스킹하여 반환
    return (SPI_readDataBlockingNonFIFO(SPI_HW_ADDR) & 0xFF);
}

void AHB_WRITE_32(uint16_t address, uint32_t data)
{
    AHB_WRITE_BURST_START(address, 1);
    AHB_WRITE_BURST_WRITE(data);
    AHB_WRITE_BURST_END();
}

uint32_t AHB_READ_32(uint16_t address)
{
    uint32_t returnData;
    AHB_READ_BURST_START(address, 1);
    returnData = AHB_READ_BURST_READ();
    AHB_READ_BURST_END();
    return returnData;
}

void AHB_WRITE_BURST_START(uint16_t address, uint8_t words)
{
    // CS Low 설정
    GPIO_writePin(SPI_CS_GPIO_PIN, 0);

    // 송신하면서 발생하는 RX 데이터는 바로 읽어서 버림 (버퍼 꼬임 방지)
    SPI_transferByte(SPIA_BASE, 0x61);                // Opcode
    SPI_transferByte(SPIA_BASE, (address >> 8));      // Addr High
    SPI_transferByte(SPIA_BASE, (address & 0xFF));    // Addr Low
    SPI_transferByte(SPIA_BASE, words);               // Words count
}

void AHB_WRITE_BURST_WRITE(uint32_t data)
{
    // 32비트 데이터를 8비트씩 4번 쪼개서 송신 (RX는 버림)
    SPI_transferByte(SPIA_BASE, (data >> 24) & 0xFF);
    SPI_transferByte(SPIA_BASE, (data >> 16) & 0xFF);
    SPI_transferByte(SPIA_BASE, (data >> 8) & 0xFF);
    SPI_transferByte(SPIA_BASE, data & 0xFF);
}

void AHB_WRITE_BURST_END(void)
{
    // CS High 설정
    GPIO_writePin(SPI_CS_GPIO_PIN, 1);
}

void AHB_READ_BURST_START(uint16_t address, uint8_t words)
{
    // CS Low 설정
    GPIO_writePin(SPI_CS_GPIO_PIN, 0);

       // 송신하면서 발생하는 RX 데이터는 바로 읽어서 버림 (버퍼 꼬임 방지)
    SPI_transferByte(SPIA_BASE, 0x61);                // Opcode
    SPI_transferByte(SPIA_BASE, (address >> 8));      // Addr High
    SPI_transferByte(SPIA_BASE, (address & 0xFF));    // Addr Low
    SPI_transferByte(SPIA_BASE, words);               // Words count
}

uint32_t AHB_READ_BURST_READ(void)
{
    uint8_t readData, readData1, readData2, readData3;
    
       uint32_t returnData = 0;
    uint32_t d0, d1, d2, d3;

    // 더미(0x00)를 보내면서 실제 데이터를 읽어옴
    d0 = SPI_transferByte(SPIA_BASE, 0x00);
    d1 = SPI_transferByte(SPIA_BASE, 0x00);
    d2 = SPI_transferByte(SPIA_BASE, 0x00);
    d3 = SPI_transferByte(SPIA_BASE, 0x00);

    returnData = (d0 << 24) | (d1 << 16) | (d2 << 8) | d3;
    return returnData;
}

void AHB_READ_BURST_END(void)
{
    // CS High 설정
    GPIO_writePin(SPI_CS_GPIO_PIN, 1);
}