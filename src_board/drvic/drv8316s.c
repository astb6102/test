
#include <math.h>

// **************************************************************************
// drivers
#include "drv8316s.h"

// **************************************************************************
// modules

// **************************************************************************
// platforms

// **************************************************************************
// the defines

// **************************************************************************
// the globals

// **************************************************************************
// the function prototypes

DRV8316_Handle DRV8316_init(void *pMemory)
{
    DRV8316_Handle handle;

    // assign the handle
    handle = (DRV8316_Handle)pMemory;

    DRV8316_resetRxTimeout(handle);
    DRV8316_resetEnableTimeout(handle);

    return(handle);
} // end of DRV8316_init() function

void DRV8316_enable(DRV8316_Handle handle)
{
    DRV8316_Obj *obj = (DRV8316_Obj *)handle;
    volatile uint16_t enableWaitTimeOut;
    uint16_t n = 0;

    // Enable the DRV8316
    GPIO_writePin(obj->gpioNumber_EN, 0);
    GPIO_writePin(obj->gpioNumber_EN, 0);

    // Wait for the DRV8316 to go through start up sequence
    for(n = 0; n < 0xffff; n++)
    {
        __asm(" NOP");
    }

    enableWaitTimeOut = 0;

    // Make sure the FAULT bit is not set during startup
    while(((DRV8316_readSPI(handle, DRV8316_ADDRESS_STATUS_0) &
            DRV8316_STAT00_FAULT_BITS) != 0) && (enableWaitTimeOut < 1000))
    {
        if(++enableWaitTimeOut > 999)
        {
            obj->enableTimeOut = true;
        }
    }

    // Wait for the DRV8316 to go through start up sequence
    for(n = 0; n < 0xffff; n++)
    {
        __asm(" NOP");
    }

    // Write 011b to this register to unlock all registers
    DRV8316_writeSPI(handle,  DRV8316_ADDRESS_CONTROL_1, 0x03);

    // Clear Fault, Slew rate is 200 V/μs
    DRV8316_writeSPI(handle,  DRV8316_ADDRESS_CONTROL_2, 0x19);

    return;
} // end of DRV8316_enable() function

void DRV8316_setSPIHandle(DRV8316_Handle handle, uint32_t spiHandle)
{
    DRV8316_Obj *obj = (DRV8316_Obj *)handle;

    // initialize the serial peripheral interface object
    obj->spiHandle = spiHandle;

    return;
} // end of DRV8316_setSPIHandle() function

void DRV8316_setGPIOCSNumber(DRV8316_Handle handle, uint32_t gpioNumber)
{
    DRV8316_Obj *obj = (DRV8316_Obj *)handle;

    // initialize the gpio interface object
    obj->gpioNumber_CS = gpioNumber;

    return;
} // end of DRV8316_setGPIOCSNumber() function

void DRV8316_setGPIOENNumber(DRV8316_Handle handle, uint32_t gpioNumber)
{
    DRV8316_Obj *obj = (DRV8316_Obj *)handle;

    // initialize the gpio interface object
    obj->gpioNumber_EN = gpioNumber;

    return;
} // end of DRV8316_setGPIOENNumber() function

void DRV8316_setupSPI(DRV8316_Handle handle,
                      DRV8316_VARS_t *drv8316Vars)
{
    DRV8316_Address_e drvRegAddr;
    uint16_t drvDataNew;

    // Set Default Values
    // Manual Read/Write
    drv8316Vars->manReadAddr  = 0;
    drv8316Vars->manReadData  = 0;
    drv8316Vars->manReadCmd = false;
    drv8316Vars->manWriteAddr = 0;
    drv8316Vars->manWriteData = 0;
    drv8316Vars->manWriteCmd = false;

    // Read/Write
    drv8316Vars->readCmd  = false;
    drv8316Vars->writeCmd = false;

    // Read registers for default values
    // Read Status Register 0
    drvRegAddr = DRV8316_ADDRESS_STATUS_0;
    drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
    drv8316Vars->statReg00.all = drvDataNew;

    // Read Status Register 1
    drvRegAddr = DRV8316_ADDRESS_STATUS_1;
    drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
    drv8316Vars->statReg01.all = drvDataNew;

    // Read Status Register 2
    drvRegAddr = DRV8316_ADDRESS_STATUS_2;
    drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
    drv8316Vars->statReg02.all = drvDataNew;

      // Read Control Register 1
    drvRegAddr = DRV8316_ADDRESS_CONTROL_1;
    drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
    drv8316Vars->ctrlReg01.all = drvDataNew;

    // Read Control Register 2
    drvRegAddr = DRV8316_ADDRESS_CONTROL_2;
    drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
    drv8316Vars->ctrlReg02.all = drvDataNew;

    // Read Control Register 3
    drvRegAddr = DRV8316_ADDRESS_CONTROL_3;
    drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
    drv8316Vars->ctrlReg03.all = drvDataNew;

    // Read Control Register 4
    drvRegAddr = DRV8316_ADDRESS_CONTROL_4;
    drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
    drv8316Vars->ctrlReg04.all = drvDataNew;

    // Read Control Register 5
    drvRegAddr = DRV8316_ADDRESS_CONTROL_5;
    drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
    drv8316Vars->ctrlReg05.all = drvDataNew;

    // Read Control Register 6
    drvRegAddr = DRV8316_ADDRESS_CONTROL_6;
    drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
    drv8316Vars->ctrlReg06.all = drvDataNew;

    // Read Control Register 10
    drvRegAddr = DRV8316_ADDRESS_CONTROL_10;
    drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
    drv8316Vars->ctrlReg10.all = drvDataNew;

    return;
} // end of DRV8316_setupSPI() function

uint16_t DRV8316_readSPI(DRV8316_Handle handle,
                         const DRV8316_Address_e regAddr)
{
    DRV8316_Obj *obj = (DRV8316_Obj *)handle;
    uint16_t ctrlWord;
    uint16_t n;
    const uint16_t data = 0;
    volatile uint16_t readWord;
    volatile uint16_t WaitTimeOut = 0;

    volatile SPI_RxFIFOLevel RxFifoCnt = SPI_FIFO_RXEMPTY;

    // build the control word
    ctrlWord = (uint16_t)DRV8316_buildCtrlWord(DRV8316_CTRLMODE_READ, regAddr, data);

#ifdef DRV_CS_GPIO
    GPIO_writePin(26, 0);
    GPIO_writePin(26, 0);
#endif  // DRV_CS_GPIO

    // wait for registers to update
    for(n = 0; n < 0x08; n++)
    {
        __asm(" NOP");
    }

    // reset the Rx fifo pointer to zero
    SPI_resetRxFIFO(obj->spiHandle);
    SPI_enableFIFO(obj->spiHandle);

    // wait for registers to update
    for(n = 0; n < 0x20; n++)
    {
        __asm(" NOP");
    }

    // write the command
    SPI_writeDataBlockingNonFIFO(obj->spiHandle, ctrlWord);

    // wait for two words to populate the RX fifo, or a wait timeout will occur
    while(RxFifoCnt < SPI_FIFO_RX1)
    {
        RxFifoCnt = SPI_getRxFIFOStatus(obj->spiHandle);

        if(++WaitTimeOut > 0xfffe)
        {
            obj->rxTimeOut = true;
        }
    }

    WaitTimeOut = 0xffff;

    // wait for registers to update
    for(n = 0; n < 0x100; n++)
    {
        __asm(" NOP");
    }

#ifdef DRV_CS_GPIO
    GPIO_writePin(26, 1);
    GPIO_writePin(26, 1);
#endif  // DRV_CS_GPIO

    // Read the word
    readWord = SPI_readDataNonBlocking(obj->spiHandle);

    return(readWord & DRV8316_DATA_MASK);
} // end of DRV8316_readSPI() function

/*
uint16_t DRV8316_readSPI(DRV8316_Handle handle,
                         const DRV8316_Address_e regAddr)
{
    DRV8316_Obj *obj = (DRV8316_Obj *)handle;
    uint16_t ctrlWord;
    uint16_t n;

    uint16_t txBuf[2];
    uint16_t rxBuf[2];

    ctrlWord = (uint16_t)DRV8316_buildCtrlWord(DRV8316_CTRLMODE_READ, regAddr, 0);

#ifdef DRV_CS_GPIO
    GPIO_writePin(obj->gpioNumber_CS, 0);
    GPIO_writePin(obj->gpioNumber_CS, 0);
#endif

    for(n = 0; n < 0x08; n++) { __asm(" NOP"); }

    // DRV8316는 응답 파이프라인/상태 포함 때문에 2워드 수신을 전제로 처리
    txBuf[0] = ctrlWord;   // read command
    txBuf[1] = 0x0000U;    // dummy (클럭 추가)

    rxBuf[0] = 0U;
    rxBuf[1] = 0U;

    // charLength=16bit, numOfWords=2
    SPI_pollingFIFOTransaction(obj->spiHandle, 16U, txBuf, rxBuf, 2U, 0U);  // [file:17]

#ifdef DRV_CS_GPIO
    GPIO_writePin(obj->gpioNumber_CS, 1);
    GPIO_writePin(obj->gpioNumber_CS, 1);
#endif

    // 두 번째 워드를 데이터로 사용 (첫 워드는 status/이전 응답 성격일 수 있음)
    return (rxBuf[1] & DRV8316_DATA_MASK);
}*/

/*
void DRV8316_writeSPI(DRV8316_Handle handle, const DRV8316_Address_e regAddr,
                      const uint16_t data)
{
    DRV8316_Obj *obj = (DRV8316_Obj *)handle;
    uint16_t ctrlWord;
    uint16_t n;

    // build the control word
    ctrlWord = (uint16_t)DRV8316_buildCtrlWord(DRV8316_CTRLMODE_WRITE, regAddr, data);

#ifdef DRV_CS_GPIO
    GPIO_writePin(26, 0);
    GPIO_writePin(26, 0);
#endif  // DRV_CS_GPIO

    // wait for GPIO
    for(n = 0; n < 0x08; n++)
    {
        __asm(" NOP");
    }

    // reset the Rx fifo pointer to zero
    SPI_resetRxFIFO(obj->spiHandle);
    SPI_enableFIFO(obj->spiHandle);

    // wait for registers to update
    for(n = 0; n < 0x40; n++)
    {
        __asm(" NOP");
    }

    // write the command
    SPI_writeDataBlockingNonFIFO(obj->spiHandle, ctrlWord);

    // wait for registers to update
    for(n = 0; n < 0x100; n++)
    {
        __asm(" NOP");
    }

#ifdef DRV_CS_GPIO
    GPIO_writePin(26, 1);
    GPIO_writePin(26, 1);
#endif  // DRV_CS_GPIO

    return;
}  // end of DRV8316_writeSPI() function
*/
void DRV8316_writeSPI(DRV8316_Handle handle,
                      const DRV8316_Address_e regAddr,
                      const uint16_t data)
{
    DRV8316_Obj *obj = (DRV8316_Obj *)handle;

    uint16_t txBuf[1];
    uint16_t rxDummy[1];

    txBuf[0] = (uint16_t)DRV8316_buildCtrlWord(DRV8316_CTRLMODE_WRITE,
                                               regAddr,
                                               (data & DRV8316_DATA_MASK));
    rxDummy[0] = 0U;

#ifdef DRV_CS_GPIO
    GPIO_writePin(obj->gpioNumber_CS, 0U);
    GPIO_writePin(obj->gpioNumber_CS, 0U);
#endif

    SPI_resetRxFIFO(obj->spiHandle);
    SPI_enableFIFO(obj->spiHandle);

    // WRITE도 FIFO 트랜잭션으로 통일(1워드 송수신)
    SPI_pollingFIFOTransaction(obj->spiHandle,
                               16U,
                               txBuf,
                               rxDummy,   // 수신값이 필요 없으면 더미 버퍼
                               1U,
                               0U);

#ifdef DRV_CS_GPIO
    GPIO_writePin(obj->gpioNumber_CS, 1U);
    GPIO_writePin(obj->gpioNumber_CS, 1U);
#endif
}

void DRV8316_writeData(DRV8316_Handle handle, DRV8316_VARS_t *drv8316Vars)
{
    DRV8316_Address_e drvRegAddr;
    uint16_t drvDataNew;

    if(drv8316Vars->writeCmd)
    {
        // Write Control Register 1
        drvRegAddr = DRV8316_ADDRESS_CONTROL_1;
        drvDataNew = drv8316Vars->ctrlReg01.all & DRV8316_DATA_MASK;
        DRV8316_writeSPI(handle, drvRegAddr, drvDataNew);

        // Write Control Register 2
        drvRegAddr = DRV8316_ADDRESS_CONTROL_2;
        drvDataNew = drv8316Vars->ctrlReg02.all & DRV8316_DATA_MASK;
        DRV8316_writeSPI(handle, drvRegAddr, drvDataNew);

        // Write Control Register 3
        drvRegAddr = DRV8316_ADDRESS_CONTROL_3;
        drvDataNew = drv8316Vars->ctrlReg03.all & DRV8316_DATA_MASK;
        DRV8316_writeSPI(handle, drvRegAddr, drvDataNew);

        // Write Control Register 4
        drvRegAddr = DRV8316_ADDRESS_CONTROL_4;
        drvDataNew = drv8316Vars->ctrlReg04.all & DRV8316_DATA_MASK;
        DRV8316_writeSPI(handle, drvRegAddr, drvDataNew);

        // Write Control Register 5
        drvRegAddr = DRV8316_ADDRESS_CONTROL_5;
        drvDataNew = drv8316Vars->ctrlReg05.all & DRV8316_DATA_MASK;
        DRV8316_writeSPI(handle, drvRegAddr, drvDataNew);

        // Write Control Register 6
        drvRegAddr = DRV8316_ADDRESS_CONTROL_6;
        drvDataNew = drv8316Vars->ctrlReg06.all & DRV8316_DATA_MASK;
        DRV8316_writeSPI(handle, drvRegAddr, drvDataNew);

        // Write Control Register 10
        drvRegAddr = DRV8316_ADDRESS_CONTROL_10;
        drvDataNew = drv8316Vars->ctrlReg10.all & DRV8316_DATA_MASK;
        DRV8316_writeSPI(handle, drvRegAddr, drvDataNew);

        drv8316Vars->writeCmd = false;
    }

    // Manual write to the DRV8316
    if(drv8316Vars->manWriteCmd)
    {
        // Custom Write
        drvRegAddr = (DRV8316_Address_e)(drv8316Vars->manWriteAddr << 11);
        drvDataNew = drv8316Vars->manWriteData;
        DRV8316_writeSPI(handle, drvRegAddr, drvDataNew);

        drv8316Vars->manWriteCmd = false;
    }

    return;
}  // end of DRV8316_writeData() function

void DRV8316_readData(DRV8316_Handle handle, DRV8316_VARS_t *drv8316Vars)
{
    DRV8316_Address_e drvRegAddr;
    uint16_t drvDataNew;

    if(drv8316Vars->readCmd)
    {
        // Read registers for default values
        // Read Status Register 0
        
        drvRegAddr = DRV8316_ADDRESS_STATUS_0;
        drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
        drv8316Vars->statReg00.all  = drvDataNew;

        // Read Status Register 1
        drvRegAddr = DRV8316_ADDRESS_STATUS_1;
        drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
        drv8316Vars->statReg01.all  = drvDataNew;

        // Read Status Register 2
        drvRegAddr = DRV8316_ADDRESS_STATUS_2;
        drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
        drv8316Vars->statReg02.all  = drvDataNew;

        // Read Control Register 1
        drvRegAddr = DRV8316_ADDRESS_CONTROL_1;
        drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
        drv8316Vars->ctrlReg01.all  = drvDataNew;

        // Read Control Register 2
        drvRegAddr = DRV8316_ADDRESS_CONTROL_2;
        drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
        drv8316Vars->ctrlReg02.all  = drvDataNew;

        // Read Control Register 3
        drvRegAddr = DRV8316_ADDRESS_CONTROL_3;
        drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
        drv8316Vars->ctrlReg03.all  = drvDataNew;

        // Read Control Register 4
        drvRegAddr = DRV8316_ADDRESS_CONTROL_4;
        drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
        drv8316Vars->ctrlReg04.all  = drvDataNew;

        // Read Control Register 5
        drvRegAddr = DRV8316_ADDRESS_CONTROL_5;
        drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
        drv8316Vars->ctrlReg05.all  = drvDataNew;

        // Read Control Register 6
        drvRegAddr = DRV8316_ADDRESS_CONTROL_6;
        drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
        drv8316Vars->ctrlReg06.all  = drvDataNew;

        // Read Control Register 10
        drvRegAddr = DRV8316_ADDRESS_CONTROL_10;
        drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
        drv8316Vars->ctrlReg10.all  = drvDataNew;

        drv8316Vars->readCmd = false;
    }

    // Manual read from the DRV8316
    if(drv8316Vars->manReadCmd)
    {
        // Custom Read
        drvRegAddr = (DRV8316_Address_e)(drv8316Vars->manReadAddr << 11);
        drvDataNew = DRV8316_readSPI(handle, drvRegAddr);
        drv8316Vars->manReadData = drvDataNew;

        drv8316Vars->manReadCmd = false;
    }

    return;
}  // end of DRV8316_readData() function


// end of file
