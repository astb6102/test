//#############################################################################
// $Copyright:
// Copyright (C) 2017-2025 Texas Instruments Incorporated - http://www.ti.com/
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the
//   distribution.
//
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//#############################################################################


//! \file   solutions/universal_motorcontrol_lab/f28003x/drivers/hal_all.c
//! \brief  Contains the various functions related to the HAL object
//!
//

//
// the includes
//
#include "user.h"

#include "device.h"
//#include "f28003x_device.h"
//#include "f28003x_dac.h"  

#include "driverlib/dac.h"
//
// drivers
//

// modules
//#include "F28003x_Device.h"  
//#include "F28003x_Spi.h" 
// platforms
#include "hal.h"
#include "hal_obj.h"

// libraries
#include "datalogIF.h"
#include "dac.h"
#include "gpio.h"

#ifdef _FLASH
#pragma CODE_SECTION(Flash_initModule, ".TI.ramfunc");
#endif

// **************************************************************************
// the defines


// **************************************************************************
// the globals
HAL_Handle    halHandle;      //!< the handle for the hardware abstraction layer
HAL_Obj       hal;            //!< the hardware abstraction layer object
#pragma DATA_SECTION(halHandle, "hal_data");
#pragma DATA_SECTION(hal, "hal_data");

uint16_t raw_position;
float angle_degrees;
static uint16_t previous_position = 0;
#define SPI_FIFO_RX4_7WORDS  SPI_FIFO_RX4_7  // F28003x 상수

// **************************************************************************
// the functions

void HAL_disableGlobalInts(HAL_Handle handle)
{
    // disable global interrupts
    Interrupt_disableMaster();

    return;
} // end of HAL_disableGlobalInts() function

void HAL_disableWdog(HAL_Handle halHandle)
{
    // disable watchdog
    SysCtl_disableWatchdog();

    return;
} // end of HAL_disableWdog() function

void HAL_enableCtrlInts(HAL_Handle handle)
{
    // enable the ADC interrupts for motor_1
    ADC_enableInterrupt(MTR1_ADC_INT_BASE, MTR1_ADC_INT_NUM);

    // enable the PIE interrupts associated with the ADC interrupts
    Interrupt_enable(MTR1_PIE_INT_NUM);    // motor_1

    return;
} // end of HAL_enableCtrlInts() function

// No HAL_enableADCIntsToTriggerCLA() in this device

void HAL_enableDebugInt(HAL_Handle handle)
{

    // enable debug events
    ERTM;

    return;
} // end of HAL_enableDebugInt() function

void HAL_enableGlobalInts(HAL_Handle handle)
{

    // enable global interrupts
    Interrupt_enableMaster();

    return;
} // end of HAL_enableGlobalInts() function


HAL_Handle HAL_init(void *pMemory,const size_t numBytes)
{
    HAL_Handle handle;
    HAL_Obj *obj;

    if(numBytes < sizeof(HAL_Obj))
    {
        return((HAL_Handle)NULL);
    }

    // assign the handle
    handle = (HAL_Handle)pMemory;

    // assign the object
    obj = (HAL_Obj *)handle;

    // Two ADC modules in this device
    // initialize the ADC handles
    obj->adcHandle[0] = ADCA_BASE;
    obj->adcHandle[1] = ADCB_BASE;
    obj->adcHandle[2] = ADCC_BASE;

    // initialize the ADC results
    obj->adcResult[0] = ADCARESULT_BASE;
    obj->adcResult[1] = ADCBRESULT_BASE;
    obj->adcResult[2] = ADCCRESULT_BASE;

    // No DAC modules in this device
    // No CLA module in this device

    // initialize SCI handle
    obj->sciHandle = SCIB_BASE;             //!< the SCIA handle // a->b syh0725

    // initialize CAN handle
    obj->canHandle = CANA_BASE;             //!< the CANA handle

    // initialize DMA handle
    obj->dmaHandle = DMA_BASE;              //!< the DMA handle
    obj->i2cHandle = I2CB_BASE;

#if defined(DATALOGF2_EN)
    // initialize DMA channel handle
    obj->dmaChHandle[0] = DMA_DATALOG1_BASE;     //!< the DMA Channel handle
    obj->dmaChHandle[1] = DMA_DATALOG2_BASE;     //!< the DMA Channel handle
#elif defined(DATALOGF4_EN) || defined(DATALOGI4_EN)
    obj->dmaChHandle[0] = DMA_DATALOG1_BASE;     //!< the DMA Channel handle
    obj->dmaChHandle[1] = DMA_DATALOG2_BASE;     //!< the DMA Channel handle
    obj->dmaChHandle[2] = DMA_DATALOG3_BASE;     //!< the DMA Channel handle
    obj->dmaChHandle[3] = DMA_DATALOG4_BASE;     //!< the DMA Channel handle
#endif  // DATALOGF2_EN | DATALOGF4_EN

    // initialize timer handles
    obj->timerHandle[0] = CPUTIMER0_BASE;
    obj->timerHandle[1] = CPUTIMER1_BASE;
    obj->timerHandle[2] = CPUTIMER2_BASE;

#if defined(EPWMDAC_MODE)
#if defined(HVMTRPFC_REV1P1)
    // initialize pwmdac handles
    obj->pwmDACHandle[0] = EPWMDAC1_BASE;
    obj->pwmDACHandle[1] = EPWMDAC2_BASE;
    obj->pwmDACHandle[2] = EPWMDAC3_BASE;
    obj->pwmDACHandle[3] = EPWMDAC4_BASE;
    // HVMTRPFC_REV1P1
#else
#error EPWMDAC is not supported on this kit!
#endif  // !HVMTRPFC_REV1P1
#endif  // EPWMDAC_MODE


    return(handle);
} // end of HAL_init() function

HAL_MTR_Handle HAL_MTR1_init(void *pMemory, const size_t numBytes)
{
    HAL_MTR_Handle handle;
    HAL_MTR_Obj *obj;

    if(numBytes < sizeof(HAL_MTR_Obj))
    {
        return((HAL_MTR_Handle)NULL);
    }

    // assign the handle
    handle = (HAL_MTR_Handle)pMemory;

    // assign the object
    obj = (HAL_MTR_Obj *)handle;

    // initialize PWM handles for Motor 1
    obj->pwmHandle[0] = MTR1_PWM_U_BASE;        //!< the PWM handle
    obj->pwmHandle[1] = MTR1_PWM_V_BASE;        //!< the PWM handle
    obj->pwmHandle[2] = MTR1_PWM_W_BASE;        //!< the PWM handle
    obj->pwmHandle[3] = MTR1_PWM_P_BASE;        //!< the PWM handle// syh
    // initialize CMPSS handle
#if defined(MOTOR1_ISBLDC)
    obj->cmpssHandle[0] = MTR1_CMPSS_IDC_BASE;  //!< the CMPSS handle
#elif defined(MOTOR1_DCLINKSS)
    obj->cmpssHandle[0] = MTR1_CMPSS_IDC_BASE;  //!< the CMPSS handle
#else   // !(MOTOR1_ISBLDC || MOTOR1_DCLINKSS)
    obj->cmpssHandle[0] = MTR1_CMPSS_U_BASE;    //!< the CMPSS handle
    obj->cmpssHandle[1] = MTR1_CMPSS_V_BASE;    //!< the CMPSS handle
    obj->cmpssHandle[2] = MTR1_CMPSS_W_BASE;    //!< the CMPSS handle
#endif  // !(MOTOR1_ISBLDC || MOTOR1_DCLINKSS)

#if defined(MOTOR1_HALL) && defined(CMD_CAP_EN)
#error HALL and CMD_CAP can't be enabled at the same time
#elif defined(MOTOR1_HALL)
    // initialize CAP handles for Motor 1
    obj->capHandle[0] = MTR1_CAP_U_BASE;        //!< the CAP handle
    obj->capHandle[1] = MTR1_CAP_V_BASE;        //!< the CAP handle
    obj->capHandle[2] = MTR1_CAP_W_BASE;        //!< the CAP handle
#elif defined(CMD_CAP_EN)
    obj->capHandle = MTR1_CAP_FREQ_BASE;        //!< the CAP handle
#endif // MOTOR1_HALL || CMD_CAP_EN

    // No PGA modules in this device

    // Assign gateEnableGPIO
#if defined(DRV8329AEVM_REVA)
    obj->gateEnableGPIO = MTR1_GATE_EN_GPIO;
    obj->gateSleepGPIO = MTR1_GATE_nSLEEP_GPIO;
    // DRV8329AEVM_REVA
#elif defined(BSXL8323RS_REVA)
    // initialize drv8323 interface
    obj->drvicHandle = DRVIC_init(&obj->drvic);

    // initialize SPI handle
    obj->spiHandle = MTR1_SPI_BASE;             //!< the SPI handle

    obj->gateEnableGPIO = MTR1_GATE_EN_GPIO;
    obj->gateCalGPIO = MTR1_GATE_CAL_GPIO;
    // BSXL8323RS_REVA
#elif defined(BSXL8323RH_REVB)
    obj->gateModeGPIO = MTR1_GATE_MODE_GPIO;
    obj->gateGainGPIO = MTR1_GATE_GAIN_GPIO;
    obj->gateCalGPIO = MTR1_GATE_CAL_GPIO;
    obj->gateEnableGPIO = MTR1_GATE_EN_GPIO;
    // BSXL8323RH_REVB
#elif defined(BSXL8353RS_REVA)
    // initialize drv8353interface
    obj->drvicHandle = DRVIC_init(&obj->drvic);

    // initialize SPI handle
    obj->spiHandle = MTR1_SPI_BASE;             //!< the SPI handle

    obj->gateEnableGPIO = MTR1_GATE_EN_GPIO;
    // BSXL8353RS_REVA
#elif defined(BSXL8316RT_REVA)
    // initialize drv8316 interface
    obj->drvicHandle = DRVIC_init(&obj->drvic);

    // initialize SPI handle
    obj->spiHandle = MTR1_SPI_BASE;             //!< the SPI handle

    obj->gateEnableGPIO = MTR1_GATE_EN_GPIO;
    obj->gateSleepGPIO = MTR1_DRV_SLEEP_GPIO;

    // enable the DRV device
    GPIO_writePin(obj->gateSleepGPIO, 0);   // 1-Active,  0-Low Power Sleep Mode

    DEVICE_DELAY_US(10.0f);                 // delay 10us

    GPIO_writePin(obj->gateEnableGPIO, 0);  // 1-Disable, 0-Enable

    DEVICE_DELAY_US(10.0f);                 // delay 10us

    GPIO_writePin(obj->gateSleepGPIO, 1);   // 1-Active,  0-Low Power Sleep Mode
    // BSXL8316RT_REVA
#elif defined(BSXL3PHGAN_REVA)
    obj->gateEnableGPIO = MTR1_GATE_EN_GPIO;
    // BSXL3PHGAN_REVA
#elif defined(HVMTRPFC_REV1P1)
    obj->gateEnableGPIO = MTR1_GATE_EN_GPIO;
    // HVMTRPFC_REV1P1
#endif  // Assign gateEnableGPIO

    // initialize QEP driver
    obj->qepHandle = MTR1_QEP_BASE;             // the QEP handle


    obj->motorNum = MTR_1;

    return(handle);
} // end of HAL_MTR1_init() function

void HAL_setParams(HAL_Handle handle)
{
    // disable global interrupts
    Interrupt_disableMaster();

#ifdef _FLASH
    //
    // Copy time critical code and flash setup code to RAM. This includes the
    // following functions: InitFlash();
    //
    // The RamfuncsLoadStart, RamfuncsLoadSize, and RamfuncsRunStart symbols
    // are created by the linker. Refer to the device .cmd file.
    //
    memcpy(&runStart_ctrlfuncs, &loadStart_ctrlfuncs, (size_t)&loadSize_ctrlfuncs);
#endif  // _FLASH


    // setup the ADCs
    HAL_setupADCs(handle);


    // Sets up the CPU timer for time base
    HAL_setupTimeBaseTimer(handle, USER_TIME_BASE_FREQ_Hz);

    // Sets up the timers for CPU usage diagnostics
    HAL_setupCPUUsageTimer(handle);

#if defined(DATALOGF4_EN) || defined(DATALOGF2_EN)
    // setup the DMA
    HAL_setupDMA();
#endif  // DATALOGF4_EN || DATALOGF2_EN

    // setup the SCI
    HAL_setupSCIA(handle);

    // setup the i2c
    HAL_setupI2CA(handle);

#if defined(EPWMDAC_MODE)
    // setup the PWM DACs
    HAL_setupPWMDACs(handle, USER_SYSTEM_FREQ_MHz);
#endif  //EPWMDAC_MODE

    return;
} // end of HAL_setParams() function

void HAL_MTR_setParams(HAL_MTR_Handle handle, USER_Params *pUserParams)
{
    HAL_setNumCurrentSensors(handle, pUserParams->numCurrentSensors);
    HAL_setNumVoltageSensors(handle, pUserParams->numVoltageSensors);

    // setup the PWMs
    HAL_setupPWMs(handle);

    // setup the CMPSSs
    HAL_setupCMPSSs(handle);

#if defined(MOTOR1_ENC)
    // setup the EQEP
    HAL_setupQEP(handle);
#endif  // MOTOR1_ENC

    // setup faults
    HAL_setupMtrFaults(handle);

#if defined(MOTOR1_HALL) || defined(CMD_CAP_EN)
    // setup the CAPs
    HAL_setupCAPs(handle);
#endif  // MOTOR1_HALL || CMD_CAP_EN

#if defined(BSXL8323RS_REVA) || defined(BSXL8353RS_REVA) || \
    defined(BSXL8316RT_REVA)

    // setup the spi for drv8323/drv8353/drv8316
    HAL_setupSPI(handle);
#endif  // BSXL8323RS_REVA || BSXL8353RS_REVA || BSXL8316RT_REVA

    // disable the PWM
    HAL_disablePWM(handle);

    return;
} // end of HAL_MTR_setParams() function

void HAL_setupADCs(HAL_Handle handle) // adc setting syh
{
    HAL_Obj *obj = (HAL_Obj *)handle;

    SysCtl_delay(100U);

#if defined(BSXL8323RS_REVA) || defined(BSXL8323RH_REVB) || \
    defined(BSXL8353RS_REVA) || defined(BSXL3PHGAN_REVA) || \
    defined(BSXL8316RT_REVA) || defined(DRV8329AEVM_REVA)
    // LAUNCHXL-F280039C based kits
    ADC_setVREF(obj->adcHandle[0], ADC_REFERENCE_INTERNAL, ADC_REFERENCE_3_3V);
    ADC_setVREF(obj->adcHandle[1], ADC_REFERENCE_INTERNAL, ADC_REFERENCE_3_3V);
    ADC_setVREF(obj->adcHandle[2], ADC_REFERENCE_INTERNAL, ADC_REFERENCE_3_3V);
#elif defined(HVMTRPFC_REV1P1)
    // TMDSCNCD280039C based kits
    ADC_setVREF(obj->adcHandle[0], ADC_REFERENCE_INTERNAL, ADC_REFERENCE_3_3V);
    ADC_setVREF(obj->adcHandle[1], ADC_REFERENCE_INTERNAL, ADC_REFERENCE_3_3V);
    ADC_setVREF(obj->adcHandle[2], ADC_REFERENCE_INTERNAL, ADC_REFERENCE_3_3V);
    // HVMTRPFC_REV1P1
#else
#error Select the right clock of PLL for the board
#endif  // ADC Reference

    SysCtl_delay(100U);

    // Set main clock scaling factor (40MHz max clock for the ADC module)
    ADC_setPrescaler(obj->adcHandle[0], ADC_CLK_DIV_2_0);
    ADC_setPrescaler(obj->adcHandle[1], ADC_CLK_DIV_2_0);
    ADC_setPrescaler(obj->adcHandle[2], ADC_CLK_DIV_2_0);

    // set the ADC interrupt pulse generation to end of conversion
    ADC_setInterruptPulseMode(obj->adcHandle[0], ADC_PULSE_END_OF_CONV);
    ADC_setInterruptPulseMode(obj->adcHandle[1], ADC_PULSE_END_OF_CONV);
    ADC_setInterruptPulseMode(obj->adcHandle[2], ADC_PULSE_END_OF_CONV);

    // set priority of SOCs
    ADC_setSOCPriority(obj->adcHandle[0], ADC_PRI_ALL_HIPRI);
    ADC_setSOCPriority(obj->adcHandle[1], ADC_PRI_ALL_HIPRI);
    ADC_setSOCPriority(obj->adcHandle[2], ADC_PRI_ALL_HIPRI);

    // enable the ADCs
    ADC_enableConverter(obj->adcHandle[0]);
    ADC_enableConverter(obj->adcHandle[1]);
    ADC_enableConverter(obj->adcHandle[2]);

    // delay to allow ADCs to power up
    SysCtl_delay(1000U);

    //-------------------------------------------------------------------------
#if defined(MOTOR1_ISBLDC)
    // configure the interrupt sources
    // Interrupt for motor 1
    ADC_setInterruptSource(MTR1_ADC_INT_BASE,
                           MTR1_ADC_INT_NUM, MTR1_ADC_INT_SOC);

    // Idc 1st
    ADC_setupSOC(MTR1_IDC1_ADC_BASE, MTR1_IDC1_ADC_SOC_NUM, MTR1_IDC_TRIGGER_SOC,
                 MTR1_IDC1_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);

    // Configure PPB to eliminate subtraction related calculation
    // PPB is associated with ADCA_SOC0
    ADC_setupPPB(MTR1_IDC1_ADC_BASE, MTR1_IDC1_ADC_PPB_NUM, MTR1_IDC1_ADC_SOC_NUM);

    // Write zero to this for now till offset calibration complete
    ADC_setPPBCalibrationOffset(MTR1_IDC1_ADC_BASE, MTR1_IDC1_ADC_PPB_NUM, 0);

    // Idc 2nd
    ADC_setupSOC(MTR1_IDC2_ADC_BASE, MTR1_IDC2_ADC_SOC_NUM, MTR1_IDC_TRIGGER_SOC,
                 MTR1_IDC2_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);

    // Configure PPB to eliminate subtraction related calculation
    // PPB is associated with ADCC_SOC0
    ADC_setupPPB(MTR1_IDC2_ADC_BASE, MTR1_IDC2_ADC_PPB_NUM, MTR1_IDC2_ADC_SOC_NUM);

    // Write zero to this for now till offset calibration complete
    ADC_setPPBCalibrationOffset(MTR1_IDC2_ADC_BASE, MTR1_IDC2_ADC_PPB_NUM, 0);

    // VSEN_A_M1
    ADC_setupSOC(MTR1_VU_ADC_BASE, MTR1_VU_ADC_SOC_NUM, MTR1_ADC_TRIGGER_SOC,
                 MTR1_VU_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);

    // VSEN_B_M1
    ADC_setupSOC(MTR1_VV_ADC_BASE, MTR1_VV_ADC_SOC_NUM, MTR1_ADC_TRIGGER_SOC,
                 MTR1_VV_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);

    // VSEN_C_M1
    ADC_setupSOC(MTR1_VW_ADC_BASE, MTR1_VW_ADC_SOC_NUM, MTR1_ADC_TRIGGER_SOC,
                 MTR1_VW_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);

    // VSEN_DCBUS_M1-->Trig Interrupt
    ADC_setupSOC(MTR1_VDC_ADC_BASE, MTR1_VDC_ADC_SOC_NUM, MTR1_ADC_TRIGGER_SOC,
                 MTR1_VDC_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);

#if defined(CMD_POT_EN) || defined(DRV8329AEVM_REVA)
    // POT_M1
    ADC_setupSOC(MTR1_POT_ADC_BASE, MTR1_POT_ADC_SOC_NUM, MTR1_ADC_TRIGGER_SOC,
                 MTR1_POT_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);
#endif  // CMD_POT_EN | DRV8329AEVM_REVA

#else // !MOTOR1_ISBLDC
    // configure the SOCs for M1
#if defined(MOTOR1_DCLINKSS)
    // configure the interrupt sources
    // Interrupt for motor 1
    ADC_setInterruptSource(MTR1_ADC_INT_BASE,
                           MTR1_ADC_INT_NUM, MTR1_ADC_INT_SOC);

    // Idc 1st
    ADC_setupSOC(MTR1_IDC1_ADC_BASE, MTR1_IDC1_ADC_SOC_NUM, MTR1_IDC1_TRIGGER_SOC,
                 MTR1_IDC1_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);

    // Configure PPB to eliminate subtraction related calculation
    // PPB is associated with ADCA_SOC0
    ADC_setupPPB(MTR1_IDC1_ADC_BASE, MTR1_IDC1_ADC_PPB_NUM, MTR1_IDC1_ADC_SOC_NUM);

    // Write zero to this for now till offset calibration complete
    ADC_setPPBCalibrationOffset(MTR1_IDC1_ADC_BASE, MTR1_IDC1_ADC_PPB_NUM, 0);

    // Idc 2nd
    ADC_setupSOC(MTR1_IDC2_ADC_BASE, MTR1_IDC2_ADC_SOC_NUM, MTR1_IDC2_TRIGGER_SOC,
                 MTR1_IDC2_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);

    // Configure PPB to eliminate subtraction related calculation
    // PPB is associated with ADCC_SOC0
    ADC_setupPPB(MTR1_IDC2_ADC_BASE, MTR1_IDC2_ADC_PPB_NUM, MTR1_IDC2_ADC_SOC_NUM);

    // Write zero to this for now till offset calibration complete
    ADC_setPPBCalibrationOffset(MTR1_IDC2_ADC_BASE, MTR1_IDC2_ADC_PPB_NUM, 0);

    // Idc 3rd
    ADC_setupSOC(MTR1_IDC3_ADC_BASE, MTR1_IDC3_ADC_SOC_NUM, MTR1_IDC3_TRIGGER_SOC,
                 MTR1_IDC3_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);

    // Configure PPB to eliminate subtraction related calculation
    // PPB is associated with ADCA_SOC0
    ADC_setupPPB(MTR1_IDC3_ADC_BASE, MTR1_IDC3_ADC_PPB_NUM, MTR1_IDC3_ADC_SOC_NUM);

    // Write zero to this for now till offset calibration complete
    ADC_setPPBCalibrationOffset(MTR1_IDC3_ADC_BASE, MTR1_IDC3_ADC_PPB_NUM, 0);

    // Idc 4th
    ADC_setupSOC(MTR1_IDC4_ADC_BASE, MTR1_IDC4_ADC_SOC_NUM, MTR1_IDC4_TRIGGER_SOC,
                 MTR1_IDC4_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);

    // Configure PPB to eliminate subtraction related calculation
    // PPB is associated with ADCC_SOC0
    ADC_setupPPB(MTR1_IDC4_ADC_BASE, MTR1_IDC4_ADC_PPB_NUM, MTR1_IDC4_ADC_SOC_NUM);

    // Write zero to this for now till offset calibration complete
    ADC_setPPBCalibrationOffset(MTR1_IDC4_ADC_BASE, MTR1_IDC4_ADC_PPB_NUM, 0);

#else   // !(MOTOR1_DCLINKSS)
    // configure the interrupt sources
    // Interrupt for motor 1
    ADC_setInterruptSource(MTR1_ADC_INT_BASE,
                           MTR1_ADC_INT_NUM, MTR1_ADC_INT_SOC);
    // ISEN_A_M1
    ADC_setupSOC(MTR1_IU_ADC_BASE, MTR1_IU_ADC_SOC_NUM, MTR1_ADC_TRIGGER_SOC,
                 MTR1_IU_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);

    // Configure PPB to eliminate subtraction related calculation
    // PPB is associated with ADCA_SOC0
    ADC_setupPPB(MTR1_IU_ADC_BASE, MTR1_IU_ADC_PPB_NUM, MTR1_IU_ADC_SOC_NUM);

    // Write zero to this for now till offset calibration complete
    ADC_setPPBCalibrationOffset(MTR1_IU_ADC_BASE, MTR1_IU_ADC_PPB_NUM, 0);

    // ISEN_B_M1
    ADC_setupSOC(MTR1_IV_ADC_BASE, MTR1_IV_ADC_SOC_NUM, MTR1_ADC_TRIGGER_SOC,
                 MTR1_IV_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);

    // Configure PPB to eliminate subtraction related calculation
    // PPB is associated with ADCC_SOC0
    ADC_setupPPB(MTR1_IV_ADC_BASE, MTR1_IV_ADC_PPB_NUM, MTR1_IV_ADC_SOC_NUM);

    // Write zero to this for now till offset calibration complete
    ADC_setPPBCalibrationOffset(MTR1_IV_ADC_BASE, MTR1_IV_ADC_PPB_NUM, 0);

    // ISEN_C_M1
    ADC_setupSOC(MTR1_IW_ADC_BASE, MTR1_IW_ADC_SOC_NUM, MTR1_ADC_TRIGGER_SOC,
                 MTR1_IW_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);

    // Configure PPB to eliminate subtraction related calculation
    // PPB is associated with ADCA_SOC0
    ADC_setupPPB(MTR1_IW_ADC_BASE, MTR1_IW_ADC_PPB_NUM, MTR1_IW_ADC_SOC_NUM);

    // Write zero to this for now till offset calibration complete
    ADC_setPPBCalibrationOffset(MTR1_IW_ADC_BASE, MTR1_IW_ADC_PPB_NUM, 0);
/////SYH

      // AH U
    ADC_setupSOC(MTR1_AHU_ADC_BASE, MTR1_AHU_ADC_SOC_NUM, MTR1_ADC_TRIGGER_SOC,
                 MTR1_AHU_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);//syh  default 1 test 2 or 1/2

    ADC_setupPPB(MTR1_AHU_ADC_BASE, MTR1_AHU_ADC_PPB_NUM, MTR1_AHU_ADC_SOC_NUM);

    // Write zero to this for now till offset calibration complete
    ADC_setPPBCalibrationOffset(MTR1_AHU_ADC_BASE, MTR1_AHU_ADC_PPB_NUM, 0);

   // AH V
    ADC_setupSOC(MTR1_AHV_ADC_BASE, MTR1_AHV_ADC_SOC_NUM, MTR1_ADC_TRIGGER_SOC,
                 MTR1_AHV_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);

    ADC_setupPPB(MTR1_AHV_ADC_BASE, MTR1_AHV_ADC_PPB_NUM, MTR1_AHV_ADC_SOC_NUM);

    // Write zero to this for now till offset calibration complete
    ADC_setPPBCalibrationOffset(MTR1_AHV_ADC_BASE, MTR1_AHV_ADC_PPB_NUM, 0);
   // AH W
    ADC_setupSOC(MTR1_AHW_ADC_BASE, MTR1_AHW_ADC_SOC_NUM, MTR1_ADC_TRIGGER_SOC,
                 MTR1_AHW_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);

    ADC_setupPPB(MTR1_AHW_ADC_BASE, MTR1_AHW_ADC_PPB_NUM, MTR1_AHW_ADC_SOC_NUM);

    // Write zero to this for now till offset calibration complete
    ADC_setPPBCalibrationOffset(MTR1_AHW_ADC_BASE, MTR1_AHW_ADC_PPB_NUM, 0);

    //ad cmd
    ADC_setupSOC(MTR1_CMD_ADC_BASE, MTR1_CMD_ADC_SOC_NUM, MTR1_ADC_TRIGGER_SOC,
                 MTR1_CMD_ADC_CH_NUM, MTR1_ADC_I_SAMPLEWINDOW);

    ADC_setupSOC(USERADC_B_BASE, USERADC_B0_SOCNUM,
            MTR1_ADC_TRIGGER_SOC, USERADC_B0_CHNUM,  MTR1_ADC_I_SAMPLEWINDOW);

    ADC_setupSOC(USERADC_B_BASE, USERADC_B2_SOCNUM,
            MTR1_ADC_TRIGGER_SOC, USERADC_B2_CHNUM,  MTR1_ADC_I_SAMPLEWINDOW);

    ADC_setupSOC(USERADC_B_BASE, USERADC_B3_SOCNUM,
           MTR1_ADC_TRIGGER_SOC, USERADC_B3_CHNUM,  MTR1_ADC_I_SAMPLEWINDOW);

    ADC_setupSOC(USERADC_B_BASE, USERADC_B4_SOCNUM,
            MTR1_ADC_TRIGGER_SOC, USERADC_B4_CHNUM,  MTR1_ADC_I_SAMPLEWINDOW);

    //ADC_setupPPB(MTR1_CMD_ADC_BASE, MTR1_CMD_ADC_PPB_NUM, MTR1_CMD_ADC_SOC_NUM); //syh ppb

    // Write zero to this for now till offset calibration complete
    //ADC_setPPBCalibrationOffset(MTR1_CMD_ADC_BASE, MTR1_CMD_ADC_PPB_NUM, 0); //syh ppb






#endif   // !(MOTOR1_DCLINKSS)

#if defined(MOTOR1_FAST) || defined(MOTOR1_ISBLDC)  //adc setup _ syh
    // VSEN_A_M1
    ADC_setupSOC(MTR1_VU_ADC_BASE, MTR1_VU_ADC_SOC_NUM, MTR1_ADC_TRIGGER_SOC,
                 MTR1_VU_ADC_CH_NUM, MTR1_ADC_V_SAMPLEWINDOW);

    // VSEN_B_M1
    ADC_setupSOC(MTR1_VV_ADC_BASE, MTR1_VV_ADC_SOC_NUM, MTR1_ADC_TRIGGER_SOC,
                 MTR1_VV_ADC_CH_NUM, MTR1_ADC_V_SAMPLEWINDOW);

    // VSEN_C_M1
    ADC_setupSOC(MTR1_VW_ADC_BASE, MTR1_VW_ADC_SOC_NUM, MTR1_ADC_TRIGGER_SOC,
                 MTR1_VW_ADC_CH_NUM, MTR1_ADC_V_SAMPLEWINDOW);
#endif  // MOTOR1_FAST || MOTOR1_ISBLDC

    // VSEN_DCBUS_M1-->Trig Interrupt
    ADC_setupSOC(MTR1_VDC_ADC_BASE, MTR1_VDC_ADC_SOC_NUM, MTR1_ADC_TRIGGER_SOC,
                 MTR1_VDC_ADC_CH_NUM, MTR1_ADC_V_SAMPLEWINDOW);

#if defined(CMD_POT_EN) || defined(DRV8329AEVM_REVA)
    // POT_M1
    ADC_setupSOC(MTR1_POT_ADC_BASE, MTR1_POT_ADC_SOC_NUM, MTR1_ADC_TRIGGER_SOC,
                 MTR1_POT_ADC_CH_NUM, MTR1_ADC_V_SAMPLEWINDOW);
#endif  // CMD_POT_EN | DRV8329AEVM_REVA
#endif  // !MOTOR1_ISBLDC

    return;
} // end of HAL_setupADCs() function

#if defined(BUFDAC_MODE) // temp. dac. syh
void HAL_setupDACs(HAL_Handle handle)
{
    HAL_Obj *obj = (HAL_Obj *)handle;

    // AIO -> A0/B15/C15/DACA_OUT Analog mode selected
    //GPIO_GPHAMSEL_GPIO231 (0x80U);

    //GPIO_GPHAMSEL_GPIO232 (0x100U)

   // GPIO_GPHAMSEL_GPIO242 (0x40000U);
    GPIO_setAnalogMode(20U,GPIO_ANALOG_ENABLED);
    obj->bufDACHandle[0]=DACA_BASE;

    // AIO -> A1/B7/DACB_OUT Analog mode selected
   // GPIO_setAnalogMode(232, GPIO_ANALOG_ENABLED); //syh 

    // AIO -> B3/VDAC, Analog mode selected
    //GPIO_setAnalogMode(21U, 1U); //syh

    // Set DAC reference voltage.
    DAC_setReferenceVoltage(obj->bufDACHandle[0], DAC_REF_ADC_VREFHI);
    //DAC_setReferenceVoltage(obj->bufDACHandle[1], DAC_REF_ADC_VREFHI);

    // Set DAC load mode.
    DAC_setLoadMode(obj->bufDACHandle[0], DAC_LOAD_SYSCLK);
   // DAC_setLoadMode(obj->bufDACHandle[1], DAC_LOAD_SYSCLK);

    // Set the DAC Gain Mode
    DAC_setGainMode(obj->bufDACHandle[0], DAC_GAIN_TWO);
   //DAC_setGainMode(obj->bufDACHandle[1], DAC_GAIN_TWO);

    // Enable the DAC output
    DAC_enableOutput(obj->bufDACHandle[0]);
    //DAC_enableOutput(obj->bufDACHandle[1]);

    // Set the DAC shadow output
    DAC_setShadowValue(obj->bufDACHandle[0], 0U);
   // DAC_setShadowValue(obj->bufDACHandle[1], 0U);

    // Delay for buffered DAC to power up.
    DEVICE_DELAY_US(500);

    return;
    /*
       HAL_Obj *obj = (HAL_Obj *)handle;
    
    // 1. GPIO 아날로그 모드 설정 (올바른 핀 번호 사용)
    GPIO_setAnalogMode(0, GPIO_ANALOG_ENABLED);  // DACA_OUT/ADCINA0
    
    // 2. DAC 핸들 초기화
    obj->bufDACHandle[0] = DACA_BASE;
    
    // 3. DAC 설정
    // Set DAC reference voltage
    DAC_setReferenceVoltage(obj->bufDACHandle[0], DAC_REF_ADC_VREFHI);
    
    // Set DAC load mode
    DAC_setLoadMode(obj->bufDACHandle[0], DAC_LOAD_SYSCLK);
    
    // Set the DAC Gain Mode
    DAC_setGainMode(obj->bufDACHandle[0], DAC_GAIN_TWO);
    
    // Enable the DAC output
    DAC_enableOutput(obj->bufDACHandle[0]);
    
    // Set the DAC shadow output
    DAC_setShadowValue(obj->bufDACHandle[0], 0U);
    
    // Delay for buffered DAC to power up
    DEVICE_DELAY_US(500);
    
    return;*/
} // end of HAL_setupDACs() function
#endif  // BUFDAC_MODE

#if defined(MOTOR1_HALL) && defined(CMD_CAP_EN)
#error HALL and CMD_CAP can't be enabled at the same time
#elif defined(MOTOR1_HALL)
void HAL_setupCAPs(HAL_MTR_Handle handle)
{
    HAL_MTR_Obj    *obj = (HAL_MTR_Obj *)handle;
    uint16_t  cnt;

    for(cnt = 0; cnt < 3; cnt++)
    {
        // Disable ,clear all capture flags and interrupts
        ECAP_disableInterrupt(obj->capHandle[cnt],
                              (ECAP_ISR_SOURCE_CAPTURE_EVENT_1  |
                               ECAP_ISR_SOURCE_CAPTURE_EVENT_2  |
                               ECAP_ISR_SOURCE_CAPTURE_EVENT_3  |
                               ECAP_ISR_SOURCE_CAPTURE_EVENT_4  |
                               ECAP_ISR_SOURCE_COUNTER_OVERFLOW |
                               ECAP_ISR_SOURCE_COUNTER_PERIOD   |
                               ECAP_ISR_SOURCE_COUNTER_COMPARE));

        ECAP_clearInterrupt(obj->capHandle[cnt],
                            (ECAP_ISR_SOURCE_CAPTURE_EVENT_1  |
                             ECAP_ISR_SOURCE_CAPTURE_EVENT_2  |
                             ECAP_ISR_SOURCE_CAPTURE_EVENT_3  |
                             ECAP_ISR_SOURCE_CAPTURE_EVENT_4  |
                             ECAP_ISR_SOURCE_COUNTER_OVERFLOW |
                             ECAP_ISR_SOURCE_COUNTER_PERIOD   |
                             ECAP_ISR_SOURCE_COUNTER_COMPARE));

        // Disable CAP1-CAP4 register loads
        ECAP_disableTimeStampCapture(obj->capHandle[cnt]);

        // Configure eCAP
        //    Enable capture mode.
        //    One shot mode, stop capture at event 3.
        //    Set polarity of the events to rising, falling, rising edge.
        //    Set capture in time difference mode.
        //    Select input from XBAR4/5/6.
        //    Enable eCAP module.
        //    Enable interrupt.
        ECAP_stopCounter(obj->capHandle[cnt]);
        ECAP_enableCaptureMode(obj->capHandle[cnt]);

        // Sets eCAP in Capture mode
        ECAP_setCaptureMode(obj->capHandle[cnt], ECAP_CONTINUOUS_CAPTURE_MODE, ECAP_EVENT_3);

        // Sets the Capture event prescaler.
        ECAP_setEventPrescaler(obj->capHandle[cnt], 0U);

        // Sets a phase shift value count.
        ECAP_setPhaseShiftCount(obj->capHandle[cnt], 0U);

        // Sets the Capture event polarity
       ECAP_setEventPolarity(obj->capHandle[cnt], ECAP_EVENT_1, ECAP_EVNT_FALLING_EDGE); //syh
        ECAP_setEventPolarity(obj->capHandle[cnt], ECAP_EVENT_2, ECAP_EVNT_RISING_EDGE);
        ECAP_setEventPolarity(obj->capHandle[cnt], ECAP_EVENT_3, ECAP_EVNT_FALLING_EDGE); //syh
        ECAP_setEventPolarity(obj->capHandle[cnt], ECAP_EVENT_4, ECAP_EVNT_RISING_EDGE);

        // Configure counter reset on events
        ECAP_enableCounterResetOnEvent(obj->capHandle[cnt], ECAP_EVENT_1);
        ECAP_enableCounterResetOnEvent(obj->capHandle[cnt], ECAP_EVENT_2);
        ECAP_enableCounterResetOnEvent(obj->capHandle[cnt], ECAP_EVENT_3);
        ECAP_enableCounterResetOnEvent(obj->capHandle[cnt], ECAP_EVENT_4);

        // Configures emulation mode.
        ECAP_setEmulationMode(obj->capHandle[cnt], ECAP_EMULATION_FREE_RUN);

        // Enable counter loading with phase shift value.
        ECAP_enableLoadCounter(obj->capHandle[cnt]);

        // Configures Sync out signal mode.
        ECAP_setSyncOutMode(obj->capHandle[cnt], ECAP_SYNC_OUT_SYNCI);

        // Set up the source for sync-in pulse
        ECAP_setSyncInPulseSource(obj->capHandle[cnt], ECAP_SYNC_IN_PULSE_SRC_DISABLE);

        // Starts Time stamp counter
        ECAP_startCounter(obj->capHandle[cnt]);

        // Enables time stamp capture
        ECAP_enableTimeStampCapture(obj->capHandle[cnt]);

        // Re-arms the eCAP module
        ECAP_reArm(obj->capHandle[cnt]);
    }

    XBAR_setInputPin(INPUTXBAR_BASE, MTR1_CAP_U_XBAR, MTR1_HALL_U_GPIO);
    XBAR_setInputPin(INPUTXBAR_BASE, MTR1_CAP_V_XBAR, MTR1_HALL_V_GPIO);
    XBAR_setInputPin(INPUTXBAR_BASE, MTR1_CAP_W_XBAR, MTR1_HALL_W_GPIO);

    ECAP_selectECAPInput(obj->capHandle[0], MTR1_CAP_U_INSEL);
    ECAP_selectECAPInput(obj->capHandle[1], MTR1_CAP_V_INSEL);
    ECAP_selectECAPInput(obj->capHandle[2], MTR1_CAP_W_INSEL);

    return;
}   // HAL_setupCAPs()

void HAL_resetCAPTimeStamp(HAL_MTR_Handle handle)
{
    HAL_MTR_Obj    *obj = (HAL_MTR_Obj *)handle;
    uint16_t  cnt;

    for(cnt = 0; cnt < 3; cnt++)
    {
        ECAP_setAPWMPeriod(obj->capHandle[cnt], 0);
        ECAP_setAPWMCompare(obj->capHandle[cnt], 0x01FFFFFF);
        ECAP_setAPWMShadowPeriod(obj->capHandle[cnt], 0x01FFFFFF);
        ECAP_setAPWMShadowCompare(obj->capHandle[cnt], 0x01FFFFFF);
    }

    return;
}   // HAL_resetCAPTimeStamp()
#elif defined(CMD_CAP_EN)
void HAL_setupCAPs(HAL_MTR_Handle handle)
{
    HAL_MTR_Obj    *obj = (HAL_MTR_Obj *)handle;

    // Disable ,clear all capture flags and interrupts
    ECAP_disableInterrupt(obj->capHandle,
                          (ECAP_ISR_SOURCE_CAPTURE_EVENT_1  |
                           ECAP_ISR_SOURCE_CAPTURE_EVENT_2  |
                           ECAP_ISR_SOURCE_CAPTURE_EVENT_3  |
                           ECAP_ISR_SOURCE_CAPTURE_EVENT_4  |
                           ECAP_ISR_SOURCE_COUNTER_OVERFLOW |
                           ECAP_ISR_SOURCE_COUNTER_PERIOD   |
                           ECAP_ISR_SOURCE_COUNTER_COMPARE));

    ECAP_clearInterrupt(obj->capHandle,
                        (ECAP_ISR_SOURCE_CAPTURE_EVENT_1  |
                         ECAP_ISR_SOURCE_CAPTURE_EVENT_2  |
                         ECAP_ISR_SOURCE_CAPTURE_EVENT_3  |
                         ECAP_ISR_SOURCE_CAPTURE_EVENT_4  |
                         ECAP_ISR_SOURCE_COUNTER_OVERFLOW |
                         ECAP_ISR_SOURCE_COUNTER_PERIOD   |
                         ECAP_ISR_SOURCE_COUNTER_COMPARE));

    // Disable CAP1-CAP4 register loads
    ECAP_disableTimeStampCapture(obj->capHandle);

    // Configure eCAP
    //    Enable capture mode.
    //    One shot mode, stop capture at event 3.
    //    Set polarity of the events to rising, falling, rising edge.
    //    Set capture in time difference mode.
    //    Select input from XBAR4/5/6.
    //    Enable eCAP module.
    //    Enable interrupt.
    ECAP_stopCounter(obj->capHandle);
    ECAP_enableCaptureMode(obj->capHandle);

    // Sets the Capture event prescaler.
    ECAP_setCaptureMode(obj->capHandle, ECAP_CONTINUOUS_CAPTURE_MODE, ECAP_EVENT_4);

    // Sets a phase shift value count.
    ECAP_setPhaseShiftCount(obj->capHandle, 0U);

    // Sets the Capture event polarity
    ECAP_setEventPolarity(obj->capHandle, ECAP_EVENT_1, ECAP_EVNT_FALLING_EDGE);
    ECAP_setEventPolarity(obj->capHandle, ECAP_EVENT_2, ECAP_EVNT_RISING_EDGE);
    ECAP_setEventPolarity(obj->capHandle, ECAP_EVENT_3, ECAP_EVNT_FALLING_EDGE);
    ECAP_setEventPolarity(obj->capHandle, ECAP_EVENT_4, ECAP_EVNT_RISING_EDGE);

    // Configure counter reset on events
    ECAP_enableCounterResetOnEvent(obj->capHandle, ECAP_EVENT_1);
    ECAP_enableCounterResetOnEvent(obj->capHandle, ECAP_EVENT_2);
    ECAP_enableCounterResetOnEvent(obj->capHandle, ECAP_EVENT_3);
    ECAP_enableCounterResetOnEvent(obj->capHandle, ECAP_EVENT_4);

    // Configures emulation mode.
    ECAP_setEmulationMode(obj->capHandle, ECAP_EMULATION_FREE_RUN);

    // Enable counter loading with phase shift value.
    ECAP_enableLoadCounter(obj->capHandle);

    // Configures Sync out signal mode.
    ECAP_setSyncOutMode(obj->capHandle, ECAP_SYNC_OUT_SYNCI);

    // Set up the source for sync-in pulse
    ECAP_setSyncInPulseSource(obj->capHandle, ECAP_SYNC_IN_PULSE_SRC_DISABLE);

    // Starts Time stamp counter
    ECAP_startCounter(obj->capHandle);

    // Enables time stamp capture
    ECAP_enableTimeStampCapture(obj->capHandle);

    // Re-arms the eCAP module
    ECAP_reArm(obj->capHandle);

    XBAR_setInputPin(INPUTXBAR_BASE, MTR1_CAP_FREQ_XBAR, MTR1_CAP_FREQ_GPIO);

    ECAP_selectECAPInput(obj->capHandle, MTR1_CAP_FREQ_INSEL);

    return;
}

void HAL_resetCAPTimeStamp(HAL_MTR_Handle handle)
{
    HAL_MTR_Obj    *obj = (HAL_MTR_Obj *)handle;

    ECAP_setAPWMPeriod(obj->capHandle, 0);
    ECAP_setAPWMCompare(obj->capHandle, 0x01FFFFFF);
    ECAP_setAPWMShadowPeriod(obj->capHandle, 0x01FFFFFF);
    ECAP_setAPWMShadowCompare(obj->capHandle, 0x01FFFFFF);

    return;
}
#endif  // MOTOR1_HALL || CMD_CAP_EN

// HAL_setupCMPSSs
void HAL_setupCMPSSs(HAL_MTR_Handle handle)
{
    HAL_MTR_Obj *obj = (HAL_MTR_Obj *)handle;

#if defined(MOTOR1_ISBLDC) || defined(MOTOR1_DCLINKSS)
    // Refer to the Technical Reference Manual to configure the ePWM X-Bar
#if defined(DRV8329AEVM_REVA)
    uint16_t cmpsaDACH = MTR1_CMPSS_DACH_VALUE;

    ASysCtl_selectCMPHPMux(MTR1_IDC_CMPHP_SEL, MTR1_IDC_CMPHP_MUX);

    // Enable CMPSS and configure the negative input signal to come from the DAC
    CMPSS_enableModule(obj->cmpssHandle[0]);

    // NEG signal from DAC for COMP-H
    CMPSS_configHighComparator(obj->cmpssHandle[0], CMPSS_INSRC_DAC);

    // Configure the output signals. Both CTRIPH and CTRIPOUTH will be fed by
    // the asynchronous comparator output.
    // Dig filter output ==> CTRIPH, Dig filter output ==> CTRIPOUTH
    CMPSS_configOutputsHigh(obj->cmpssHandle[0],
                            CMPSS_TRIP_FILTER |
                            CMPSS_TRIPOUT_FILTER);

    // Configure digital filter. For this example, the maxiumum values will be
    // used for the clock prescale, sample window size, and threshold.
    CMPSS_configFilterHigh(obj->cmpssHandle[0], 32, 32, 30);
    CMPSS_initFilterHigh(obj->cmpssHandle[0]);

    // Set up COMPHYSCTL register
    // COMP hysteresis set to 2x typical value
    CMPSS_setHysteresis(obj->cmpssHandle[0], 1);

    // Use VDDA as the reference for the DAC and set DAC value to midpoint for
    // arbitrary reference
    CMPSS_configDAC(obj->cmpssHandle[0],
               CMPSS_DACREF_VDDA | CMPSS_DACVAL_SYSCLK | CMPSS_DACSRC_SHDW);

    // Set DAC-H to allowed MAX +ve current
    CMPSS_setDACValueHigh(obj->cmpssHandle[0], cmpsaDACH);

    // Clear any high comparator digital filter output latch
    CMPSS_clearFilterLatchHigh(obj->cmpssHandle[0]);
    // DRV8329AEVM_REVA
#elif defined(BSXL8323RS_REVA) || defined(BSXL8323RH_REVB)
    uint16_t cmpsaDACL = MTR1_CMPSS_DACL_VALUE;

    ASysCtl_selectCMPLPMux(MTR1_IDC_CMPLP_SEL, MTR1_IDC_CMPLP_MUX);

    // Enable CMPSS and configure the negative input signal to come from the DAC
    CMPSS_enableModule(obj->cmpssHandle[0]);

    // NEG signal from DAC for COMP-L
    CMPSS_configLowComparator(obj->cmpssHandle[0], CMPSS_INSRC_DAC);

    // Configure the output signals. Both CTRIPH/L and CTRIPOUTH/L will be fed by
    // the asynchronous comparator output.
    // Dig filter output ==> CTRIPL, Dig filter output ==> CTRIPOUTL
    CMPSS_configOutputsLow(obj->cmpssHandle[0],
                           CMPSS_TRIP_FILTER |
                           CMPSS_TRIPOUT_FILTER |
                           CMPSS_INV_INVERTED);

    // Configure digital filter. For this example, the maxiumum values will be
    // used for the clock prescale, sample window size, and threshold.
    // Initialize the filter logic and start filtering
    CMPSS_configFilterLow(obj->cmpssHandle[0], 32, 32, 30);
    CMPSS_initFilterLow(obj->cmpssHandle[0]);

    // Set up COMPHYSCTL register
    // COMP hysteresis set to 2x typical value
    CMPSS_setHysteresis(obj->cmpssHandle[0], 1);

    // Use VDDA as the reference for the DAC and set DAC value to midpoint for
    // arbitrary reference
    CMPSS_configDAC(obj->cmpssHandle[0],
               CMPSS_DACREF_VDDA | CMPSS_DACVAL_SYSCLK | CMPSS_DACSRC_SHDW);

    // Set DAC-L to allowed MAX -ve current
    CMPSS_setDACValueLow(obj->cmpssHandle[0], cmpsaDACL);

    // Clear any low comparator digital filter output latch
    CMPSS_clearFilterLatchLow(obj->cmpssHandle[0]);
    // BSXL8323RS_REVA | BSXL8323RH_REVB
#else
#error This board doesn't support single shunt
#endif  // BSXL8323RS_REVA || BSXL8323RH_REVB
#else   // !(MOTOR1_ISBLDC || MOTOR1_DCLINKSS)
    // Refer to the Table 9-2 in Chapter 9 of TMS320F28004x
    // Technical Reference Manual (SPRUI33B), to configure the ePWM X-Bar
    uint16_t cmpsaDACH = MTR1_CMPSS_DACH_VALUE;
    uint16_t cmpsaDACL = MTR1_CMPSS_DACL_VALUE;

#if defined(HVMTRPFC_REV1P1) || defined(BSXL8316RT_REVA) || \
    defined(BSXL3PHGAN_REVA)
    //
    ASysCtl_selectCMPHPMux(MTR1_IU_CMPHP_SEL, MTR1_IU_CMPHP_MUX);
    ASysCtl_selectCMPLPMux(MTR1_IU_CMPLP_SEL, MTR1_IU_CMPLP_MUX);

    ASysCtl_selectCMPHPMux(MTR1_IV_CMPHP_SEL, MTR1_IV_CMPHP_MUX);
    ASysCtl_selectCMPLPMux(MTR1_IV_CMPLP_SEL, MTR1_IV_CMPLP_MUX);

    ASysCtl_selectCMPHPMux(MTR1_IW_CMPHP_SEL, MTR1_IW_CMPHP_MUX);
    ASysCtl_selectCMPLPMux(MTR1_IW_CMPLP_SEL, MTR1_IW_CMPLP_MUX);

    //----- U Phase ------------------------------------------------------------
    // Enable CMPSS and configure the negative input signal to come from the DAC
    CMPSS_enableModule(obj->cmpssHandle[0]);

    // NEG signal from DAC for COMP-H
    CMPSS_configHighComparator(obj->cmpssHandle[0], CMPSS_INSRC_DAC);

    // NEG signal from DAC for COMP-L
    CMPSS_configLowComparator(obj->cmpssHandle[0], CMPSS_INSRC_DAC);

    // Configure the output signals. Both CTRIPH and CTRIPOUTH will be fed by
    // the asynchronous comparator output.
    // Dig filter output ==> CTRIPH, Dig filter output ==> CTRIPOUTH
    CMPSS_configOutputsHigh(obj->cmpssHandle[0],
                            CMPSS_TRIP_FILTER |
                            CMPSS_TRIPOUT_FILTER);

    // Dig filter output ==> CTRIPL, Dig filter output ==> CTRIPOUTL
    CMPSS_configOutputsLow(obj->cmpssHandle[0],
                           CMPSS_TRIP_FILTER |
                           CMPSS_TRIPOUT_FILTER |
                           CMPSS_INV_INVERTED);

    // Configure digital filter. For this example, the maxiumum values will be
    // used for the clock prescale, sample window size, and threshold.
    CMPSS_configFilterHigh(obj->cmpssHandle[0], 32, 32, 30);
    CMPSS_initFilterHigh(obj->cmpssHandle[0]);

    // Initialize the filter logic and start filtering
    CMPSS_configFilterLow(obj->cmpssHandle[0], 32, 32, 30);
    CMPSS_initFilterLow(obj->cmpssHandle[0]);

    // Set up COMPHYSCTL register
    // COMP hysteresis set to 2x typical value
    CMPSS_setHysteresis(obj->cmpssHandle[0], 1);

    // Use VDDA as the reference for the DAC and set DAC value to midpoint for
    // arbitrary reference
    CMPSS_configDAC(obj->cmpssHandle[0],
               CMPSS_DACREF_VDDA | CMPSS_DACVAL_SYSCLK | CMPSS_DACSRC_SHDW);

    // Set DAC-H to allowed MAX +ve current
    CMPSS_setDACValueHigh(obj->cmpssHandle[0], cmpsaDACH);

    // Set DAC-L to allowed MAX -ve current
    CMPSS_setDACValueLow(obj->cmpssHandle[0], cmpsaDACL);

    // Clear any high comparator digital filter output latch
    CMPSS_clearFilterLatchHigh(obj->cmpssHandle[0]);

    // Clear any low comparator digital filter output latch
    CMPSS_clearFilterLatchLow(obj->cmpssHandle[0]);

    //---------------- V pHase -------------------------------------------------
    // Enable CMPSS and configure the negative input signal to come from the DAC
    CMPSS_enableModule(obj->cmpssHandle[1]);

    // NEG signal from DAC for COMP-H
    CMPSS_configHighComparator(obj->cmpssHandle[1], CMPSS_INSRC_DAC);

    // NEG signal from DAC for COMP-L
    CMPSS_configLowComparator(obj->cmpssHandle[1], CMPSS_INSRC_DAC);

    // Configure the output signals. Both CTRIPH and CTRIPOUTH will be fed by
    // the asynchronous comparator output.
    // Dig filter output ==> CTRIPH, Dig filter output ==> CTRIPOUTH
    CMPSS_configOutputsHigh(obj->cmpssHandle[1],
                            CMPSS_TRIP_FILTER |
                            CMPSS_TRIPOUT_FILTER);

    // Dig filter output ==> CTRIPL, Dig filter output ==> CTRIPOUTL
    CMPSS_configOutputsLow(obj->cmpssHandle[1],
                           CMPSS_TRIP_FILTER |
                           CMPSS_TRIPOUT_FILTER |
                           CMPSS_INV_INVERTED);

    // Configure digital filter. For this example, the maxiumum values will be
    // used for the clock prescale, sample window size, and threshold.
    CMPSS_configFilterHigh(obj->cmpssHandle[1], 32, 32, 30);
    CMPSS_initFilterHigh(obj->cmpssHandle[1]);

    // Initialize the filter logic and start filtering
    CMPSS_configFilterLow(obj->cmpssHandle[1], 32, 32, 30);
    CMPSS_initFilterLow(obj->cmpssHandle[1]);

    // Set up COMPHYSCTL register
    // COMP hysteresis set to 2x typical value
    CMPSS_setHysteresis(obj->cmpssHandle[1], 1);

    // Use VDDA as the reference for the DAC and set DAC value to midpoint for
    // arbitrary reference
    CMPSS_configDAC(obj->cmpssHandle[1],
               CMPSS_DACREF_VDDA | CMPSS_DACVAL_SYSCLK | CMPSS_DACSRC_SHDW);

    // Set DAC-H to allowed MAX +ve current
    CMPSS_setDACValueHigh(obj->cmpssHandle[1], cmpsaDACH);

    // Set DAC-L to allowed MAX -ve current
    CMPSS_setDACValueLow(obj->cmpssHandle[1], cmpsaDACL);

    // Clear any high comparator digital filter output latch
    CMPSS_clearFilterLatchHigh(obj->cmpssHandle[1]);

    // Clear any low comparator digital filter output latch
    CMPSS_clearFilterLatchLow(obj->cmpssHandle[1]);

    //---------------- W Phase -------------------------------------------------
    // Enable CMPSS and configure the negative input signal to come from the DAC
    CMPSS_enableModule(obj->cmpssHandle[2]);

    // NEG signal from DAC for COMP-H
    CMPSS_configHighComparator(obj->cmpssHandle[2], CMPSS_INSRC_DAC);

    // NEG signal from DAC for COMP-L
    CMPSS_configLowComparator(obj->cmpssHandle[2], CMPSS_INSRC_DAC);

    // Configure the output signals. Both CTRIPH and CTRIPOUTH will be fed by
    // the asynchronous comparator output.
    // Dig filter output ==> CTRIPH, Dig filter output ==> CTRIPOUTH
    CMPSS_configOutputsHigh(obj->cmpssHandle[2],
                            CMPSS_TRIP_FILTER |
                            CMPSS_TRIPOUT_FILTER);

    // Dig filter output ==> CTRIPL, Dig filter output ==> CTRIPOUTL
    CMPSS_configOutputsLow(obj->cmpssHandle[2],
                           CMPSS_TRIP_FILTER |
                           CMPSS_TRIPOUT_FILTER |
                           CMPSS_INV_INVERTED);

    // Configure digital filter. For this example, the maxiumum values will be
    // used for the clock prescale, sample window size, and threshold.
    CMPSS_configFilterHigh(obj->cmpssHandle[2], 32, 32, 30);
    CMPSS_initFilterHigh(obj->cmpssHandle[2]);

    // Initialize the filter logic and start filtering
    CMPSS_configFilterLow(obj->cmpssHandle[2], 32, 32, 30);
    CMPSS_initFilterLow(obj->cmpssHandle[2]);

    // Set up COMPHYSCTL register
    // COMP hysteresis set to 2x typical value
    CMPSS_setHysteresis(obj->cmpssHandle[2], 1);

    // Use VDDA as the reference for the DAC and set DAC value to midpoint for
    // arbitrary reference
    CMPSS_configDAC(obj->cmpssHandle[2],
               CMPSS_DACREF_VDDA | CMPSS_DACVAL_SYSCLK | CMPSS_DACSRC_SHDW);

    // Set DAC-H to allowed MAX +ve current
    CMPSS_setDACValueHigh(obj->cmpssHandle[2], cmpsaDACH);

    // Set DAC-L to allowed MAX -ve current
    CMPSS_setDACValueLow(obj->cmpssHandle[2], cmpsaDACL);

    // Clear any high comparator digital filter output latch
    CMPSS_clearFilterLatchHigh(obj->cmpssHandle[2]);

    // Clear any low comparator digital filter output latch
    CMPSS_clearFilterLatchLow(obj->cmpssHandle[2]);

#elif defined(BSXL8323RH_REVB) || defined(BSXL8323RS_REVA) || \
      defined(BSXL8353RS_REVA)
    ASysCtl_selectCMPHPMux(MTR1_IU_CMPHP_SEL, MTR1_IU_CMPHP_MUX);

    ASysCtl_selectCMPHPMux(MTR1_IV_CMPHP_SEL, MTR1_IV_CMPHP_MUX);
    ASysCtl_selectCMPLPMux(MTR1_IV_CMPLP_SEL, MTR1_IV_CMPLP_MUX);

    ASysCtl_selectCMPLPMux(MTR1_IW_CMPLP_SEL, MTR1_IW_CMPLP_MUX);

    //----- U Phase ------------------------------------------------------------
    // Enable CMPSS and configure the negative input signal to come from the DAC
    CMPSS_enableModule(obj->cmpssHandle[0]);

    // NEG signal from DAC for COMP-H
    CMPSS_configHighComparator(obj->cmpssHandle[0], CMPSS_INSRC_DAC);

    // NEG signal from DAC for COMP-L
    CMPSS_configLowComparator(obj->cmpssHandle[0], CMPSS_INSRC_DAC);

    // Configure the output signals. Both CTRIPH and CTRIPOUTH will be fed by
    // the asynchronous comparator output.
    // Dig filter output ==> CTRIPH, Dig filter output ==> CTRIPOUTH
    CMPSS_configOutputsHigh(obj->cmpssHandle[0],
                            CMPSS_TRIP_FILTER |
                            CMPSS_TRIPOUT_FILTER);

    // Configure digital filter. For this example, the maxiumum values will be
    // used for the clock prescale, sample window size, and threshold.
    CMPSS_configFilterHigh(obj->cmpssHandle[0], 32, 32, 30);
    CMPSS_initFilterHigh(obj->cmpssHandle[0]);

    // Set up COMPHYSCTL register
    // COMP hysteresis set to 2x typical value
    CMPSS_setHysteresis(obj->cmpssHandle[0], 1);

    // Use VDDA as the reference for the DAC and set DAC value to midpoint for
    // arbitrary reference
    CMPSS_configDAC(obj->cmpssHandle[0],
               CMPSS_DACREF_VDDA | CMPSS_DACVAL_SYSCLK | CMPSS_DACSRC_SHDW);

    // Set DAC-H to allowed MAX +ve current
    CMPSS_setDACValueHigh(obj->cmpssHandle[0], cmpsaDACH);

    // Clear any high comparator digital filter output latch
    CMPSS_clearFilterLatchHigh(obj->cmpssHandle[0]);


    //---------------- V pHase -------------------------------------------------
    // Enable CMPSS and configure the negative input signal to come from the DAC
    CMPSS_enableModule(obj->cmpssHandle[1]);

    // NEG signal from DAC for COMP-H
    CMPSS_configHighComparator(obj->cmpssHandle[1], CMPSS_INSRC_DAC);

    // NEG signal from DAC for COMP-L
    CMPSS_configLowComparator(obj->cmpssHandle[1], CMPSS_INSRC_DAC);

    // Configure the output signals. Both CTRIPH and CTRIPOUTH will be fed by
    // the asynchronous comparator output.
    // Dig filter output ==> CTRIPH, Dig filter output ==> CTRIPOUTH
    CMPSS_configOutputsHigh(obj->cmpssHandle[1],
                            CMPSS_TRIP_FILTER |
                            CMPSS_TRIPOUT_FILTER);

    // Dig filter output ==> CTRIPL, Dig filter output ==> CTRIPOUTL
    CMPSS_configOutputsLow(obj->cmpssHandle[1],
                           CMPSS_TRIP_FILTER |
                           CMPSS_TRIPOUT_FILTER |
                           CMPSS_INV_INVERTED);

    // Configure digital filter. For this example, the maxiumum values will be
    // used for the clock prescale, sample window size, and threshold.
    CMPSS_configFilterHigh(obj->cmpssHandle[1], 32, 32, 30);
    CMPSS_initFilterHigh(obj->cmpssHandle[1]);

    // Initialize the filter logic and start filtering
    CMPSS_configFilterLow(obj->cmpssHandle[1], 32, 32, 30);
    CMPSS_initFilterLow(obj->cmpssHandle[1]);

    // Set up COMPHYSCTL register
    // COMP hysteresis set to 2x typical value
    CMPSS_setHysteresis(obj->cmpssHandle[1], 1);

    // Use VDDA as the reference for the DAC and set DAC value to midpoint for
    // arbitrary reference
    CMPSS_configDAC(obj->cmpssHandle[1],
               CMPSS_DACREF_VDDA | CMPSS_DACVAL_SYSCLK | CMPSS_DACSRC_SHDW);

    // Set DAC-H to allowed MAX +ve current
    CMPSS_setDACValueHigh(obj->cmpssHandle[1], cmpsaDACH);

    // Set DAC-L to allowed MAX -ve current
    CMPSS_setDACValueLow(obj->cmpssHandle[1], cmpsaDACL);

    // Clear any high comparator digital filter output latch
    CMPSS_clearFilterLatchHigh(obj->cmpssHandle[1]);

    // Clear any low comparator digital filter output latch
    CMPSS_clearFilterLatchLow(obj->cmpssHandle[1]);

    //---------------- W Phase -------------------------------------------------
    // Enable CMPSS and configure the negative input signal to come from the DAC
    CMPSS_enableModule(obj->cmpssHandle[2]);

    // NEG signal from DAC for COMP-L
    CMPSS_configLowComparator(obj->cmpssHandle[2], CMPSS_INSRC_DAC);

    // Configure the output signals. Both CTRIPH and CTRIPOUTH will be fed by
    // the asynchronous comparator output.
    // Dig filter output ==> CTRIPL, Dig filter output ==> CTRIPOUTL
    CMPSS_configOutputsLow(obj->cmpssHandle[2],
                           CMPSS_TRIP_FILTER |
                           CMPSS_TRIPOUT_FILTER |
                           CMPSS_INV_INVERTED);

    // Configure digital filter. For this example, the maxiumum values will be
    // used for the clock prescale, sample window size, and threshold.
    // Initialize the filter logic and start filtering
    CMPSS_configFilterLow(obj->cmpssHandle[2], 32, 32, 30);
    CMPSS_initFilterLow(obj->cmpssHandle[2]);

    // Set up COMPHYSCTL register
    // COMP hysteresis set to 2x typical value
    CMPSS_setHysteresis(obj->cmpssHandle[2], 1);

    // Use VDDA as the reference for the DAC and set DAC value to midpoint for
    // arbitrary reference
    CMPSS_configDAC(obj->cmpssHandle[2],
               CMPSS_DACREF_VDDA | CMPSS_DACVAL_SYSCLK | CMPSS_DACSRC_SHDW);

    // Set DAC-L to allowed MAX -ve current
    CMPSS_setDACValueLow(obj->cmpssHandle[2], cmpsaDACL);

    // Clear any low comparator digital filter output latch
    CMPSS_clearFilterLatchLow(obj->cmpssHandle[2]);
#else
#error Not selec a right board
#endif  // !HVMTRPFC_REV1P1
#endif  // !(MOTOR1_ISBLDC || MOTOR1_DCLINKSS)

    return;
} // end of HAL_setupCMPSSs() function

// HAL_setupGate & HAL_enableDRV
#if defined(DRV8329AEVM_REVA)
void HAL_enableDRV(HAL_MTR_Handle handle)
{
    HAL_MTR_Obj *obj = (HAL_MTR_Obj *)handle;

    // Set nSLEEP to low for clear the fault
    GPIO_writePin(obj->gateSleepGPIO, 0);

    DEVICE_DELAY_US(2.0f);      // delay 2us

    // Set EN_GATE to low for enabling the DRV
    GPIO_writePin(obj->gateEnableGPIO, 0);

    DEVICE_DELAY_US(2.0f);      // 2.0us

    // Set nSLEEP to high for wake the device
    GPIO_writePin(obj->gateSleepGPIO, 1);

    DEVICE_DELAY_US(2.0f);      // delay 2us

    // Set EN_GATE to low for enabling the DRV
    GPIO_writePin(obj->gateEnableGPIO, 0);

    return;
} // HAL_setupGate() function
// DRV8329AEVM_REVA
#elif defined(BSXL8323RS_REVA) || defined(BSXL8353RS_REVA) || \
    defined(BSXL8316RT_REVA)
void HAL_enableDRV(HAL_MTR_Handle handle)
{
    HAL_MTR_Obj *obj = (HAL_MTR_Obj *)handle;

    DRVIC_enable(obj->drvicHandle);

    DEVICE_DELAY_US(1000.0f);      // delay 1000us

    return;
}  // end of HAL_enableDRV() function

void HAL_writeDRVData(HAL_MTR_Handle handle, DRVIC_VARS_t *drvicVars)
{
    HAL_MTR_Obj  *obj = (HAL_MTR_Obj *)handle;

    DRVIC_writeData(obj->drvicHandle, drvicVars);

    return;
}  // end of HAL_writeDRVData() function

void HAL_readDRVData(HAL_MTR_Handle handle, DRVIC_VARS_t *drvicVars)
{
    HAL_MTR_Obj  *obj = (HAL_MTR_Obj *)handle;

    DRVIC_readData(obj->drvicHandle, drvicVars);

    return;
}  // end of HAL_readDRVData() function


void HAL_setupDRVSPI(HAL_MTR_Handle handle, DRVIC_VARS_t *drvicVars)
{
    HAL_MTR_Obj  *obj = (HAL_MTR_Obj *)handle;

    DRVIC_setupSPI(obj->drvicHandle, drvicVars);

    return;
}  // end of HAL_setupDRVSPI() function

void HAL_setupGate(HAL_MTR_Handle handle)
{
    HAL_MTR_Obj *obj = (HAL_MTR_Obj *)handle;

    DRVIC_setSPIHandle(obj->drvicHandle, obj->spiHandle);

    DRVIC_setGPIOCSNumber(obj->drvicHandle, MTR1_DRV_SPI_CS_GPIO);
    DRVIC_setGPIOENNumber(obj->drvicHandle, MTR1_GATE_EN_GPIO);

    return;
} // HAL_setupGate() function

void HAL_setupSPI(HAL_MTR_Handle handle)
{
    HAL_MTR_Obj   *obj = (HAL_MTR_Obj *)handle;

    // Must put SPI into reset before configuring it
    SPI_disableModule(obj->spiHandle);

    // SPI configuration. Use a 500kHz SPICLK and 16-bit word size, 30MHz LSPCLK //SPI_PROT_POL0PHA0
    SPI_setConfig(obj->spiHandle, DEVICE_LSPCLK_FREQ, SPI_PROT_POL0PHA0,
                  SPI_MODE_MASTER, 500000, 16);

    SPI_disableLoopback(obj->spiHandle);

    SPI_setEmulationMode(obj->spiHandle, SPI_EMULATION_FREE_RUN);

    SPI_enableFIFO(obj->spiHandle);
    SPI_setFIFOInterruptLevel(obj->spiHandle, SPI_FIFO_TX0, SPI_FIFO_RX0);//SPI_FIFO_TX0, SPI_FIFO_RX0
    SPI_setTxFifoTransmitDelay(obj->spiHandle, 0x10);

    SPI_clearInterruptStatus(obj->spiHandle, SPI_INT_TXFF);

    // Configuration complete. Enable the module.
    SPI_enableModule(obj->spiHandle);
    // setup spiB for encoder ## syh
    SPI_disableModule(SPIB_BASE); // 설정 전 SPI 비활성화

    SPI_setConfig(SPIB_BASE,DEVICE_LSPCLK_FREQ,SPI_PROT_POL1PHA1,SPI_MODE_MASTER,3000000,16);//1000000-> *1//엔코더 속도 //new board

    SPI_disableLoopback(SPIB_BASE);
    SPI_setEmulationMode(SPIB_BASE, SPI_EMULATION_FREE_RUN);
    SPI_enableFIFO(SPIB_BASE);

    SPI_enableModule(SPIB_BASE); // 설정 후 SPI 활성화



    return;
}  // end of HAL_setupSPI() function

void HAL_setupSPIB_Encoder(void)
{
    SPI_disableModule(SPIB_BASE);
    DEVICE_DELAY_US(10.0f);

    SPI_setConfig(SPIB_BASE, DEVICE_LSPCLK_FREQ,
                  SPI_PROT_POL1PHA1, SPI_MODE_MASTER, 3000000, 16);

    SPI_disableLoopback(SPIB_BASE);
    SPI_setEmulationMode(SPIB_BASE, SPI_EMULATION_FREE_RUN);
    SPI_enableFIFO(SPIB_BASE);

    // RX 레벨 설정은 제거
    SPI_clearInterruptStatus(SPIB_BASE, SPI_INT_TXFF);
    SPI_enableModule(SPIB_BASE);
}

/*
uint16_t ReadEncoderSSI(void)
{
    uint16_t encoderData;
   // DINT;
    // SPIB 레지스터 베이스 직접 접근
    // SPIFFRX 레지스터 offset = 0x0021, RXFFST = bit[12:8]
    volatile uint16_t *pSPIFFRX = (volatile uint16_t *)(SPIB_BASE + 0x0021U);
     
    // 1) RX FIFO 비우기 (잔여 데이터 제거)
    while( ((*pSPIFFRX) & 0x1F00U) != 0U )
    {
        (void)SPI_readDataNonBlocking(SPIB_BASE);
    }

    // 2) 새 프레임 송신
    SPI_writeDataBlockingNonFIFO(SPIB_BASE, 0x0000);

    // 3) RX FIFO에 1워드 들어올 때까지 대기
   // while( ((*pSPIFFRX) & 0x1F00U) == 0U )
    //{
   //     ;
   // }
     
    // 4) 데이터 읽기
    encoderData = SPI_readDataNonBlocking(SPIB_BASE);
    //EINT;
    // 5) 13bit 위치값 추출 (RMB20SC 기준)
    return (encoderData >> 2) & 0x1FFF;
}*/
uint16_t ReadEncoderSSI(void) {
    uint16_t encoderData = 0;
    
    
    
    
    SPI_writeDataBlockingNonFIFO(SPIB_BASE, 0x0000);
   //DEVICE_DELAY_US(1);
    encoderData = SPI_readDataNonBlocking(SPIB_BASE);
    //DEVICE_DELAY_US(1);
    
   
    
    return (encoderData >> 2) & 0x1FFF;   // 13비트 마스킹
}


// 13비트 데이터를 각도로 변환
float convert_to_angle(uint16_t raw_data)
{
    float angle;
    
    // 13비트 해상도 (8192 positions per revolution)
    angle = ((float)raw_data / 8192.0) * 360.0;
    
    return angle;
}

void process_encoder_data(void)
{
    uint16_t raw_position;
    float angle_degrees;
    static uint16_t previous_position = 0;
    
    // 엔코더 데이터 읽기
    raw_position = read_rmb20sc_data();
    
   
    angle_degrees = convert_to_angle(raw_position);
    
   
    if(abs(raw_position - previous_position) > 100)
    {
        // 급격한 변화 감지 - 통신 오류 가능성
        // 에러 처리 로직 구현
    }
    
    previous_position = raw_position;
    

}




// BSXL8323RS_REVA || BSXL8353RS_REVA ||
// BSXL8316RT_REVA
#elif defined(BSXL8323RH_REVB)
void HAL_enableDRV(HAL_MTR_Handle handle)
{
    HAL_MTR_Obj *obj = (HAL_MTR_Obj *)handle;

    // Set EN_GATE to high for enabling the DRV
    GPIO_writePin(obj->gateEnableGPIO, 1);

    // Set MODE to low for setting 6-PWM mode
    GPIO_writePin(obj->gateModeGPIO, 0);

    // disable calibrate mode
    GPIO_writePin(obj->gateCalGPIO, 0);

    return;
} // HAL_setupGate() function
// BSXL8323RH_REVB
#elif defined(BSXL3PHGAN_REVA)
void HAL_enableDRV(HAL_MTR_Handle handle)
{
    HAL_MTR_Obj *obj = (HAL_MTR_Obj *)handle;

    // Set EN_GATE (nEN_uC) to low for enabling the DRV
    GPIO_writePin(obj->gateEnableGPIO, 0);

    return;
} // HAL_setupGate() function
// BSXL3PHGAN_REVA
#elif defined(HVMTRPFC_REV1P1)
void HAL_enableDRV(HAL_MTR_Handle handle)
{
    HAL_MTR_Obj *obj = (HAL_MTR_Obj *)handle;

    // Set EN_GATE to low for enabling the DRV
    GPIO_writePin(obj->gateEnableGPIO, 0);

    return;
} // HAL_setupGate() function
// HVMTRPFC_REV1P1
#else
#error No HAL_setupGate or HAL_enableDRV
#endif  // HAL_setupGate & HAL_enableDRV

void HAL_setupMtrFaults(HAL_MTR_Handle handle)
{
    HAL_MTR_Obj *obj = (HAL_MTR_Obj *)handle;
    uint16_t cnt;

    // Configure TRIP 7 to OR the High and Low trips from both
    // comparator 5, 3 & 1, clear everything first
    EALLOW;
    HWREG(XBAR_EPWM_CFG_REG_BASE + MTR1_XBAR_TRIP_ADDRL) = 0;
    HWREG(XBAR_EPWM_CFG_REG_BASE + MTR1_XBAR_TRIP_ADDRH) = 0;
    EDIS;

#if defined(MOTOR1_ISBLDC) || defined(MOTOR1_DCLINKSS)
    // Configure TRIP7 to be CTRIP5H and CTRIP5L using the ePWM X-BAR
    XBAR_setEPWMMuxConfig(MTR1_XBAR_TRIP, MTR1_IDC_XBAR_EPWM_MUX);

    // Disable all the mux first
    XBAR_disableEPWMMux(MTR1_XBAR_TRIP, 0xFFFF);

    // Enable Mux 0  OR Mux 4 to generate TRIP
    XBAR_enableEPWMMux(MTR1_XBAR_TRIP, MTR1_IDC_XBAR_MUX);
#else   // !(MOTOR1_ISBLDC || MOTOR1_DCLINKSS)
    // Configure TRIP7 to be CTRIP5H and CTRIP5L using the ePWM X-BAR
    XBAR_setEPWMMuxConfig(MTR1_XBAR_TRIP, MTR1_IU_XBAR_EPWM_MUX);

    // Configure TRIP7 to be CTRIP1H and CTRIP1L using the ePWM X-BAR
    XBAR_setEPWMMuxConfig(MTR1_XBAR_TRIP, MTR1_IV_XBAR_EPWM_MUX);

    // Configure TRIP7 to be CTRIP3H and CTRIP3L using the ePWM X-BAR
    XBAR_setEPWMMuxConfig(MTR1_XBAR_TRIP, MTR1_IW_XBAR_EPWM_MUX);

    // Disable all the mux first
    XBAR_disableEPWMMux(MTR1_XBAR_TRIP, 0xFFFF);

    // Enable Mux 0  OR Mux 4 to generate TRIP
    XBAR_enableEPWMMux(MTR1_XBAR_TRIP, MTR1_IU_XBAR_MUX |
                                       MTR1_IV_XBAR_MUX | MTR1_IW_XBAR_MUX);
#endif  // !(MOTOR1_ISBLDC || MOTOR1_DCLINKSS)

    // configure the input x bar for TZ2 to GPIO, where Over Current is connected
    XBAR_setInputPin(INPUTXBAR_BASE, MTR1_XBAR_INPUT1, MTR1_PM_nFAULT_GPIO);
    XBAR_lockInput(INPUTXBAR_BASE, MTR1_XBAR_INPUT1);

    for(cnt=0;cnt<4;cnt++) // 4 syh
    {
        EPWM_enableTripZoneSignals(obj->pwmHandle[cnt], MTR1_TZ_OSHT1);
    }

    // Configure Trip Mechanism for the Motor control software
    // -Cycle by cycle trip on CPU halt
    // -One shot fault trip zone
    // These trips need to be repeated for EPWM1 ,2 & 3

    for(cnt=0; cnt<4; cnt++) // syh
    {
        EPWM_enableTripZoneSignals(obj->pwmHandle[cnt],
                                   EPWM_TZ_SIGNAL_CBC6);

        //enable DC TRIP combinational input
        EPWM_enableDigitalCompareTripCombinationInput(obj->pwmHandle[cnt],
                                              MTR1_DCTRIPIN, EPWM_DC_TYPE_DCAH);

        EPWM_enableDigitalCompareTripCombinationInput(obj->pwmHandle[cnt],
                                              MTR1_DCTRIPIN, EPWM_DC_TYPE_DCBH);

        // Trigger event when DCAH is High
        EPWM_setTripZoneDigitalCompareEventCondition(obj->pwmHandle[cnt],
                                                     EPWM_TZ_DC_OUTPUT_A1,
                                                     EPWM_TZ_EVENT_DCXH_HIGH);

        // Trigger event when DCBH is High
        EPWM_setTripZoneDigitalCompareEventCondition(obj->pwmHandle[cnt],
                                                     EPWM_TZ_DC_OUTPUT_B1,
                                                     EPWM_TZ_EVENT_DCXL_HIGH);

        // Configure the DCA path to be un-filtered and asynchronous
        EPWM_setDigitalCompareEventSource(obj->pwmHandle[cnt],
                                          EPWM_DC_MODULE_A,
                                          EPWM_DC_EVENT_1,
                                          EPWM_DC_EVENT_SOURCE_FILT_SIGNAL);

        // Configure the DCB path to be un-filtered and asynchronous
        EPWM_setDigitalCompareEventSource(obj->pwmHandle[cnt],
                                          EPWM_DC_MODULE_B,
                                          EPWM_DC_EVENT_1,
                                          EPWM_DC_EVENT_SOURCE_FILT_SIGNAL);

        EPWM_setDigitalCompareEventSyncMode(obj->pwmHandle[cnt],
                                            EPWM_DC_MODULE_A,
                                            EPWM_DC_EVENT_1,
                                            EPWM_DC_EVENT_INPUT_NOT_SYNCED);

        EPWM_setDigitalCompareEventSyncMode(obj->pwmHandle[cnt],
                                            EPWM_DC_MODULE_B,
                                            EPWM_DC_EVENT_1,
                                            EPWM_DC_EVENT_INPUT_NOT_SYNCED);

        // Enable DCA as OST
        EPWM_enableTripZoneSignals(obj->pwmHandle[cnt], EPWM_TZ_SIGNAL_DCAEVT1);

        // Enable DCB as OST
        EPWM_enableTripZoneSignals(obj->pwmHandle[cnt], EPWM_TZ_SIGNAL_DCBEVT1);

        // What do we want the OST/CBC events to do?
        // TZA events can force EPWMxA
        // TZB events can force EPWMxB
        EPWM_setTripZoneAction(obj->pwmHandle[cnt],
                               EPWM_TZ_ACTION_EVENT_TZA,
                               EPWM_TZ_ACTION_LOW);

        EPWM_setTripZoneAction(obj->pwmHandle[cnt],
                               EPWM_TZ_ACTION_EVENT_TZB,
                               EPWM_TZ_ACTION_LOW);
    }

#if defined(MOTOR1_ISBLDC) || defined(MOTOR1_DCLINKSS)
#if defined(DRV8329AEVM_REVA)
    // Clear any low comparator digital filter output latch
    CMPSS_clearFilterLatchHigh(obj->cmpssHandle[0]);
    // DRV8329AEVM_REVA
#else
#error This board doesn't support single shunt
#endif  // BSXL8323RS_REVA || BSXL8323RH_REVB || DRV8329AEVM_REVA || HVMTRPFC_REV1P1
#else   // !(MOTOR1_ISBLDC || MOTOR1_DCLINKSS)
#if defined(BSXL8323RS_REVA) || defined(BSXL8323RH_REVB) || \
    defined(BSXL8353RS_REVA)

    // Clear any high comparator digital filter output latch
    CMPSS_clearFilterLatchHigh(obj->cmpssHandle[0]);

    // Clear any high comparator digital filter output latch
    CMPSS_clearFilterLatchHigh(obj->cmpssHandle[1]);

    // Clear any low comparator digital filter output latch
    CMPSS_clearFilterLatchLow(obj->cmpssHandle[1]);

    // Clear any low comparator digital filter output latch
    CMPSS_clearFilterLatchLow(obj->cmpssHandle[2]);

    // Clear any spurious fault
    EPWM_clearTripZoneFlag(obj->pwmHandle[0], HAL_TZFLAG_INTERRUPT_ALL);
    EPWM_clearTripZoneFlag(obj->pwmHandle[1], HAL_TZFLAG_INTERRUPT_ALL);
    EPWM_clearTripZoneFlag(obj->pwmHandle[2], HAL_TZFLAG_INTERRUPT_ALL);
    // BSXL8323RS_REVA | BSXL8323RH_REVB | BSXL8353RS_REVA
#elif defined(HVMTRPFC_REV1P1) || defined(BSXL8316RT_REVA) || \
      defined(BSXL3PHGAN_REVA)
    for(cnt=0; cnt<4; cnt++) // 4 syh
    {
        // Clear any high comparator digital filter output latch
        CMPSS_clearFilterLatchHigh(obj->cmpssHandle[cnt]);

        // Clear any low comparator digital filter output latch
        CMPSS_clearFilterLatchLow(obj->cmpssHandle[cnt]);

        // Clear any spurious fault
        EPWM_clearTripZoneFlag(obj->pwmHandle[cnt], HAL_TZFLAG_INTERRUPT_ALL);
    }
    // HVMTRPFC_REV1P1 | BSXL8316RT_REVA | BSXL8316RT_REVA
#else
#error Not Select a Right Board
#endif  // Hardware Board
#endif  // !(MOTOR1_ISBLDC || MOTOR1_DCLINKSS)

    // Clear any spurious fault
    EPWM_clearTripZoneFlag(obj->pwmHandle[0], HAL_TZFLAG_INTERRUPT_ALL);
    EPWM_clearTripZoneFlag(obj->pwmHandle[1], HAL_TZFLAG_INTERRUPT_ALL);
    EPWM_clearTripZoneFlag(obj->pwmHandle[2], HAL_TZFLAG_INTERRUPT_ALL);
     EPWM_clearTripZoneFlag(obj->pwmHandle[3], HAL_TZFLAG_INTERRUPT_ALL); // syh

    return;
} // end of HAL_setupMtrFaults() function

void HAL_setupGPIOs(HAL_Handle handle)/// GPIO SETTING
{

#if defined(DRV8329AEVM_REVA)
    // GPIO0->EPWM1A->M1_UH
    GPIO_setPinConfig(GPIO_0_EPWM1_A);
    GPIO_setDirectionMode(0, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(0, GPIO_PIN_TYPE_STD);

    // GPIO1->EPWM1B->M1_UL
    GPIO_setPinConfig(GPIO_1_EPWM1_B);
    GPIO_setDirectionMode(1, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(1, GPIO_PIN_TYPE_STD);

    // GPIO2->EPWM2A->M1_VH*
    GPIO_setPinConfig(GPIO_2_EPWM2_A);
    GPIO_setDirectionMode(2, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(2, GPIO_PIN_TYPE_STD);

    // GPIO3->EPWM2B->M1_VL*
    GPIO_setPinConfig(GPIO_3_EPWM2_B);
    GPIO_setDirectionMode(3, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(3, GPIO_PIN_TYPE_STD);

#if defined(CMD_CAN_EN)
    // GPIO4->CAN_TX
    GPIO_setPinConfig(GPIO_4_CANA_TX);
    GPIO_setDirectionMode(4, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_STD);
#else   // !CMD_CAN_EN
    // GPIO4->M2_nFAULT
    GPIO_setPinConfig(GPIO_4_GPIO4);
    GPIO_setDirectionMode(4, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_PULLUP);
#endif  // !CMD_CAN_EN

#if defined(CMD_CAN_EN)
    // GPIO5->CAN_RX
    GPIO_setPinConfig(GPIO_5_CANA_RX);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);
#else   // !CMD_CAN_EN
    // GPIO5->LPD_LED
    GPIO_setPinConfig(GPIO_5_GPIO5);
    GPIO_writePin(5, 0);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);
#endif  // !(DAC128S_ENABLE && DAC128S_SPIA) & !CMD_CAN_EN

    // GPIO6->Reserve
    GPIO_setPinConfig(GPIO_6_GPIO6);
    GPIO_setDirectionMode(6, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(6, GPIO_PIN_TYPE_STD);

    // GPIO7->Reserve
    GPIO_setPinConfig(GPIO_7_GPIO7);
    GPIO_setDirectionMode(7, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(7, GPIO_PIN_TYPE_STD);

    // GPIO8->Reserve
    GPIO_setPinConfig(GPIO_8_GPIO8);
    GPIO_setDirectionMode(8, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(8, GPIO_PIN_TYPE_STD);

    // GPIO9->Reserve GPIO
    GPIO_setPinConfig(GPIO_9_GPIO9);
    GPIO_setDirectionMode(9, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(9, GPIO_PIN_TYPE_STD);

    // GPIO10->EPWM6A->M1_WH*
    GPIO_setPinConfig(GPIO_10_EPWM6_A);
    GPIO_setDirectionMode(10, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(10, GPIO_PIN_TYPE_STD);

    // GPIO11->EPWM6B->M1_WL*
    GPIO_setPinConfig(GPIO_11_EPWM6_B);
    GPIO_setDirectionMode(11, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(11, GPIO_PIN_TYPE_STD);

    // GPIO12->Reserve
    GPIO_setPinConfig(GPIO_12_GPIO12);
    GPIO_setDirectionMode(12, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(12, GPIO_PIN_TYPE_STD);

    // GPIO13->Reserve
    GPIO_setPinConfig(GPIO_13_GPIO13);
    GPIO_setDirectionMode(13, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(13, GPIO_PIN_TYPE_STD);

    // GPIO14->HALL_U (J13)
    GPIO_setPinConfig(GPIO_14_GPIO14);
    GPIO_setDirectionMode(14, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(14, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(14, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(14, 2);

#if defined(SFRA_ENABLE)
    // GPIO15->SCIA_RX for SFRA
    GPIO_setPinConfig(GPIO_15_SCIB_RX);
    GPIO_setDirectionMode(15, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(15, GPIO_PIN_TYPE_STD);
#else  // !SFRA_ENABLE
    // GPIO15->Reserve
    GPIO_setPinConfig(GPIO_15_GPIO15);
    GPIO_setDirectionMode(15, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(15, GPIO_PIN_TYPE_STD);
#endif

    // GPIO16->Reserve
    GPIO_setPinConfig(GPIO_16_GPIO16);
    GPIO_setDirectionMode(16, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(16, GPIO_PIN_TYPE_STD);

    // GPIO17->Reserve
    GPIO_setPinConfig(GPIO_17_GPIO17);
    GPIO_setDirectionMode(17, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(17, GPIO_PIN_TYPE_STD);

    // GPIO18->Reserve
    GPIO_setPinConfig(GPIO_18_GPIO18);
    GPIO_setDirectionMode(18, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(18, GPIO_PIN_TYPE_STD);

    // GPIO19->Reserve
    GPIO_setPinConfig(GPIO_19_GPIO19);
    GPIO_setDirectionMode(19, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(19, GPIO_PIN_TYPE_STD);

    // GPIO20->LPD_LED
    GPIO_setAnalogMode(20, GPIO_ANALOG_DISABLED);
    GPIO_setPinConfig(GPIO_20_GPIO20);
    GPIO_writePin(20, 1);
    GPIO_setDirectionMode(20, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(20, GPIO_PIN_TYPE_STD);

    // GPIO21->Reserve
    GPIO_setAnalogMode(21, GPIO_ANALOG_DISABLED);
    GPIO_setPinConfig(GPIO_21_GPIO21);
    GPIO_setDirectionMode(21, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(21, GPIO_PIN_TYPE_STD);

    // GPIO22->LPD_LED
    GPIO_setPinConfig(GPIO_22_GPIO22);
    GPIO_writePin(22, 1);
    GPIO_setDirectionMode(22, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(22, GPIO_PIN_TYPE_STD);

    // GPIO23->Reserve
    GPIO_setPinConfig(GPIO_23_GPIO23);
    GPIO_writePin(23, 1);
    GPIO_setDirectionMode(23, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(23, GPIO_PIN_TYPE_STD);

    // GPIO24->Reserve
    GPIO_setPinConfig(GPIO_24_GPIO24);
    GPIO_setDirectionMode(24, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(24, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(24, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(24, 2);

    // GPIO25->Reserve
    GPIO_setPinConfig(GPIO_25_GPIO25);
    GPIO_setDirectionMode(25, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(25, GPIO_PIN_TYPE_STD);

    // GPIO26->Reserve
    GPIO_setPinConfig(GPIO_26_GPIO26);
    GPIO_setDirectionMode(26, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(26, GPIO_PIN_TYPE_STD);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO27->SPIB_STE->DAC128S_SYNC
    GPIO_setPinConfig(GPIO_27_SPIB_STE);
    GPIO_setDirectionMode(27, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(27, GPIO_PIN_TYPE_STD);
#else
    // GPIO27->Reserve
    GPIO_setPinConfig(GPIO_27_GPIO27);
    GPIO_setDirectionMode(27, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(27, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIB

    // GPIO28->HAL_GPIO_ISR_M1
    GPIO_setPinConfig(GPIO_28_GPIO28);
    GPIO_writePin(28, 1);
    GPIO_setDirectionMode(28, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(28, GPIO_PIN_TYPE_PULLUP);

    // GPIO29->M1_DRV_nSLEEP
    GPIO_setPinConfig(GPIO_29_GPIO29);
    GPIO_writePin(29, 1);
    GPIO_setDirectionMode(29, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(29, GPIO_PIN_TYPE_PULLUP);

    // GPIO30->Reserve
    GPIO_setPinConfig(GPIO_30_GPIO30);
    GPIO_setDirectionMode(30, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(30, GPIO_PIN_TYPE_STD);

    // GPIO31->Reserve
    GPIO_setPinConfig(GPIO_31_GPIO31);
    GPIO_setDirectionMode(31, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(31, GPIO_PIN_TYPE_STD);

    // GPIO32->M1_DRV_nFAULT
    GPIO_setPinConfig(GPIO_32_GPIO32);
    GPIO_setDirectionMode(32, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(32, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(32, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(32, 2);

    // GPIO33->Reserve
    GPIO_setPinConfig(GPIO_33_GPIO33);
    GPIO_setDirectionMode(33, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(33, GPIO_PIN_TYPE_STD);

    // GPIO34->Reserve
    GPIO_setPinConfig(GPIO_34_GPIO34);
    GPIO_setDirectionMode(34, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(34, GPIO_PIN_TYPE_STD);

    GPIO_setPinConfig(GPIO_35_GPIO35);
    GPIO_setDirectionMode(35, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(35, GPIO_PIN_TYPE_STD);

    // GPIO37->M1_DRV_EN
    GPIO_setPinConfig(GPIO_37_GPIO37);
    GPIO_writePin(37, 0);
    GPIO_setDirectionMode(37, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(37, GPIO_PIN_TYPE_STD);

    // GPIO39->HAL_GPIO_ISR_M1
    GPIO_setPinConfig(GPIO_39_GPIO39);
    GPIO_setDirectionMode(39, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(39, GPIO_PIN_TYPE_STD);

    // GPIO40->QEP1_A (J12)
    GPIO_setPinConfig(GPIO_40_EQEP1_A);
    GPIO_setDirectionMode(40, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(40, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(40, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(40, 2);

    // GPIO41->QEP1_B (J12)
    GPIO_setPinConfig(GPIO_41_EQEP1_B);
    GPIO_setDirectionMode(41, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(41, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(41, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(41, 2);

    // GPIO42->Reserve
    GPIO_setPinConfig(GPIO_42_GPIO42);
    GPIO_setDirectionMode(42, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(42, GPIO_PIN_TYPE_STD);

    // GPIO43->Reserve
    GPIO_setPinConfig(GPIO_43_GPIO43);
    GPIO_setDirectionMode(43, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(43, GPIO_PIN_TYPE_STD);

    // GPIO44->Reserve
    GPIO_setPinConfig(GPIO_44_GPIO44);
    GPIO_setDirectionMode(44, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(44, GPIO_PIN_TYPE_STD);

    // GPIO45->Reserve
    GPIO_setPinConfig(GPIO_45_GPIO45);
    GPIO_setDirectionMode(45, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(45, GPIO_PIN_TYPE_STD);

    // GPIO46->Reserve
    GPIO_setPinConfig(GPIO_46_GPIO46);
    GPIO_setDirectionMode(46, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(46, GPIO_PIN_TYPE_STD);

    // GPIO47->Reserve
    GPIO_setPinConfig(GPIO_47_GPIO47);
    GPIO_setDirectionMode(47, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(47, GPIO_PIN_TYPE_STD);

    // GPIO48->Reserve
    GPIO_setPinConfig(GPIO_48_GPIO48);
    GPIO_setDirectionMode(48, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(48, GPIO_PIN_TYPE_PULLUP);

    // GPIO51->Reserve
    GPIO_setPinConfig(GPIO_51_GPIO51);
    GPIO_setDirectionMode(51, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(51, GPIO_PIN_TYPE_STD);

    // GPIO55->HALL_V (J13)
    GPIO_setPinConfig(GPIO_55_GPIO55);
    GPIO_setDirectionMode(55, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(55, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(55, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(55, 2);

#if defined(SFRA_ENABLE)
    // GPIO56->SCIA_RX for SFRA
    GPIO_setPinConfig(GPIO_56_SCIB_TX);
    GPIO_setDirectionMode(56, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(56, GPIO_PIN_TYPE_STD);
#else  // !SFRA_ENABLE
    // GPIO56->Reserve
    GPIO_setPinConfig(GPIO_56_GPIO56);
    GPIO_setDirectionMode(56, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(56, GPIO_PIN_TYPE_STD);
#endif

    // GPIO57->HALL_W (J13)
    GPIO_setPinConfig(GPIO_57_GPIO57);
    GPIO_setDirectionMode(57, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(57, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(57, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(57, 2);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO58->SPIB_CLK->DAC128S_SCLK
    GPIO_setPinConfig(GPIO_58_SPIB_CLK);
    GPIO_setDirectionMode(58, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(58, GPIO_PIN_TYPE_STD);
#else
    // GPIO58->Reserve GPIO
    GPIO_setPinConfig(GPIO_58_GPIO58);
    GPIO_setDirectionMode(58, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(58, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIB

    // GPIO59->QEP1_INDEX (J12)
    GPIO_setPinConfig(GPIO_59_EQEP1_INDEX);
    GPIO_setDirectionMode(59, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(59, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(59, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(59, 2);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO60->SPIB_SIMO->DAC128S_SDI
    GPIO_setPinConfig(GPIO_60_SPIB_SIMO);
    GPIO_setDirectionMode(60, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(60, GPIO_PIN_TYPE_STD);
#else   // DAC128S_ENABLE && DAC128S_SPIB
    // GPIO60->Reserve
    GPIO_setPinConfig(GPIO_60_GPIO60);
    GPIO_setDirectionMode(60, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(60, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIB

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO61->SPIB_SOMI->DAC128S_SDO
    GPIO_setPinConfig(GPIO_61_SPIB_SOMI);
    GPIO_setDirectionMode(61, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(61, GPIO_PIN_TYPE_STD);
#else   // DAC128S_ENABLE && DAC128S_SPIB
    // GPIO61->Reserve
    GPIO_setPinConfig(GPIO_61_GPIO61);
    GPIO_setDirectionMode(61, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(61, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIB

    // end of DRV8329AEVM_REVA
//------------------------------------------------------------------------------
#elif defined(BSXL8323RS_REVA)
    // GPIO0->EPWM1A->M1_UH*
    GPIO_setPinConfig(GPIO_0_EPWM1_A);
    GPIO_setDirectionMode(0, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(0, GPIO_PIN_TYPE_STD);

    // GPIO1->EPWM1B->M1_UL*
    GPIO_setPinConfig(GPIO_1_EPWM1_B);
    GPIO_setDirectionMode(1, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(1, GPIO_PIN_TYPE_STD);

    // GPIO2->EPWM2A->M1_VH*
    GPIO_setPinConfig(GPIO_2_EPWM2_A);
    GPIO_setDirectionMode(2, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(2, GPIO_PIN_TYPE_STD);

    // GPIO3->EPWM2B->M1_VL*
    GPIO_setPinConfig(GPIO_3_EPWM2_B);
    GPIO_setDirectionMode(3, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(3, GPIO_PIN_TYPE_STD);

#if defined(CMD_CAN_EN)
    // GPIO4->CAN_TX
    GPIO_setPinConfig(GPIO_4_CANA_TX);
    GPIO_setDirectionMode(4, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_STD);
#else   // !CMD_CAN_EN
    // GPIO4->Reserve
    GPIO_setPinConfig(GPIO_4_GPIO4);
    GPIO_setDirectionMode(4, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_STD);
#endif  // !CMD_CAN_EN

#if defined(CMD_CAN_EN)
    // GPIO5->CAN_RX
    GPIO_setPinConfig(GPIO_5_CANA_RX);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);
#elif !defined(DRV_CS_GPIO) || defined(DAC128S_SPIA)
    // GPIO5->M1_DRV_SCS->Use a Jumper connect to J2-12
    GPIO_setPinConfig(GPIO_5_SPIA_STE);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_PULLUP);
#else
    // GPIO5->Reserve
    GPIO_setPinConfig(GPIO_5_GPIO5);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);
#endif  // CMD_CAN_EN)

    // GPIO6->Reserve
    GPIO_setPinConfig(GPIO_6_GPIO6);
    GPIO_setDirectionMode(6, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(6, GPIO_PIN_TYPE_STD);

    // GPIO7->Reserve
    GPIO_setPinConfig(GPIO_7_GPIO7);
    GPIO_setDirectionMode(7, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(7, GPIO_PIN_TYPE_STD);

    // GPIO8->SPIA_SIMO->M1_DRV_SDI or DAC12S_SDI
    GPIO_setPinConfig(GPIO_8_SPIA_SIMO);
    GPIO_setDirectionMode(8, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(8, GPIO_PIN_TYPE_PULLUP);

    // GPIO9->SPIA_CLK->M1_DRV_SCLK or DAC128S_SCLK
    GPIO_setPinConfig(GPIO_9_SPIA_CLK);
    GPIO_setDirectionMode(9, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(9, GPIO_PIN_TYPE_PULLUP);

    // GPIO10->EPWM6A->M1_WH*
    GPIO_setPinConfig(GPIO_10_EPWM6_A);
    GPIO_setDirectionMode(10, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(10, GPIO_PIN_TYPE_STD);

    // GPIO11->EPWM6B->M1_WL*
    GPIO_setPinConfig(GPIO_11_EPWM6_B);
    GPIO_setDirectionMode(11, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(11, GPIO_PIN_TYPE_STD);

    // GPIO12->Reserve
    GPIO_setPinConfig(GPIO_12_GPIO12);
    GPIO_setDirectionMode(12, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(12, GPIO_PIN_TYPE_STD);

    // GPIO13->Reserve
    GPIO_setPinConfig(GPIO_13_GPIO13);
    GPIO_setDirectionMode(13, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(13, GPIO_PIN_TYPE_STD);

    // GPIO14->HALL_U
    GPIO_setPinConfig(GPIO_14_GPIO14);
    GPIO_setDirectionMode(14, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(14, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(14, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(14, 2);

#if defined(SFRA_ENABLE)
    // GPIO15->SCIA_RX for SFRA
    GPIO_setPinConfig(GPIO_15_SCIB_RX);
    GPIO_setDirectionMode(15, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(15, GPIO_PIN_TYPE_STD);
#else  // !SFRA_ENABLE
    // GPIO15->Reserve
    GPIO_setPinConfig(GPIO_15_GPIO15);
    GPIO_setDirectionMode(15, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(15, GPIO_PIN_TYPE_STD);
#endif

    // GPIO16->Reserve
    GPIO_setPinConfig(GPIO_16_GPIO16);
    GPIO_setDirectionMode(16, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(16, GPIO_PIN_TYPE_STD);

    // GPIO17->SPIA_SOMI->M1_DRV_SDO or DAC12S_SDO
    GPIO_setPinConfig(GPIO_17_SPIA_SOMI);
    GPIO_setDirectionMode(17, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(17, GPIO_PIN_TYPE_PULLUP);

    // GPIO18->Reserve
    GPIO_setPinConfig(GPIO_18_GPIO18);
    GPIO_setDirectionMode(18, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(18, GPIO_PIN_TYPE_STD);

    // GPIO19->Reserve
    GPIO_setPinConfig(GPIO_19_GPIO19);
    GPIO_setDirectionMode(19, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(19, GPIO_PIN_TYPE_STD);

    // GPIO20->LP_LED-RED
    GPIO_setAnalogMode(20, GPIO_ANALOG_DISABLED);
    GPIO_setPinConfig(GPIO_20_GPIO20);
    GPIO_writePin(20, 0);
    GPIO_setDirectionMode(20, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(20, GPIO_PIN_TYPE_STD);

    // GPIO21->M2_DRV_MODE->6-PWM-Mode (->GND)
    GPIO_setAnalogMode(21, GPIO_ANALOG_DISABLED);
    GPIO_setPinConfig(GPIO_21_GPIO21);
    GPIO_writePin(21, 0);
    GPIO_setDirectionMode(21, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(21, GPIO_PIN_TYPE_STD);

    // GPIO22->M1_DRV_nSCS(GPIO Mode)
#if defined(DRV_CS_GPIO)
    GPIO_setPinConfig(GPIO_22_GPIO22);
    GPIO_writePin(22, 1);
    GPIO_setDirectionMode(22, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(22, GPIO_PIN_TYPE_STD);
#else
    GPIO_setPinConfig(GPIO_22_GPIO22);
    GPIO_setDirectionMode(22, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(22, GPIO_PIN_TYPE_STD);
#endif

    // GPIO23->M1_DRV_CAL (Low)
    GPIO_setPinConfig(GPIO_23_GPIO23);
    GPIO_writePin(23, 0);
    GPIO_setDirectionMode(23, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(23, GPIO_PIN_TYPE_STD);

    // GPIO24->M1_DRV_nFAULT*
    GPIO_setPinConfig(GPIO_24_GPIO24);
    GPIO_setDirectionMode(24, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(24, GPIO_PIN_TYPE_PULLUP);

    // GPIO25->Reserve
    GPIO_setPinConfig(GPIO_25_GPIO25);
    GPIO_setDirectionMode(25, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(25, GPIO_PIN_TYPE_PULLUP);

    // GPIO26->M2_DRV_SCS(GPIO Mode)
    GPIO_setPinConfig(GPIO_26_GPIO26);
    GPIO_writePin(26, 1);
    GPIO_setDirectionMode(26, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(26, GPIO_PIN_TYPE_STD);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO27->SPIB_STE->DAC128S_SYNC
    GPIO_setPinConfig(GPIO_27_SPIB_STE);
    GPIO_setDirectionMode(27, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(27, GPIO_PIN_TYPE_STD);
#else  // !DACEXT_ENABLE
    // GPIO27->Reserve
    GPIO_setPinConfig(GPIO_27_GPIO27);
    GPIO_setDirectionMode(27, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(27, GPIO_PIN_TYPE_STD);
#endif  // !DACEXT_ENABLE

    // GPIO28->HAL_GPIO_ISR_M1
    GPIO_setPinConfig(GPIO_28_GPIO28);
    GPIO_writePin(28, 1);
    GPIO_setDirectionMode(28, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(28, GPIO_PIN_TYPE_PULLUP);

    // GPIO29->M1_DRV_ENABLE*
    GPIO_setPinConfig(GPIO_29_GPIO29);
    GPIO_writePin(29, 1);
    GPIO_setDirectionMode(29, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(29, GPIO_PIN_TYPE_PULLUP);

    // GPIO30->Reserve
    GPIO_setPinConfig(GPIO_30_GPIO30);
    GPIO_setDirectionMode(30, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(30, GPIO_PIN_TYPE_STD);

    // GPIO31->M2_LED
    GPIO_setPinConfig(GPIO_31_GPIO31);
    GPIO_writePin(31, 0);
    GPIO_setDirectionMode(31, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(31, GPIO_PIN_TYPE_STD);

    // GPIO32->Reserve
    GPIO_setPinConfig(GPIO_32_GPIO32);
    GPIO_setDirectionMode(32, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(32, GPIO_PIN_TYPE_STD);

    // GPIO33->M1_DRV_MODE->6-PWM-Mode (->GND)
    GPIO_setPinConfig(GPIO_33_GPIO33);
    GPIO_writePin(33, 0);
    GPIO_setDirectionMode(33, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(33, GPIO_PIN_TYPE_STD);

    // GPIO34->Reserve
    GPIO_setPinConfig(GPIO_34_GPIO34);
    GPIO_setDirectionMode(34, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(34, GPIO_PIN_TYPE_STD);

#if !defined(MOTOR2_ISBLDC)
    // GPIO35->EPWM5B->M2_WL
    GPIO_setPinConfig(GPIO_35_EPWM5_B);
    GPIO_setDirectionMode(35, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(35, GPIO_PIN_TYPE_STD);
#else
    GPIO_setPinConfig(GPIO_35_GPIO35);
    GPIO_setDirectionMode(35, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(35, GPIO_PIN_TYPE_STD);
#endif // !MOTOR2_ISBLDC

    // GPIO37->Reserve
    GPIO_setPinConfig(GPIO_37_GPIO37);
    GPIO_setDirectionMode(37, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(37, GPIO_PIN_TYPE_STD);

    // GPIO39->HAL_GPIO_ISR_M1
    GPIO_setPinConfig(GPIO_39_GPIO39);
    GPIO_setDirectionMode(39, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(39, GPIO_PIN_TYPE_STD);

    // GPIO40->QEP1_A (J12)
    GPIO_setPinConfig(GPIO_40_EQEP1_A);
    GPIO_setDirectionMode(40, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(40, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(40, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(40, 2);

    // GPIO41->QEP1_B (J12)
    GPIO_setPinConfig(GPIO_41_EQEP1_B);
    GPIO_setDirectionMode(41, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(41, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(41, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(41, 2);

    // GPIO42->M1_DRV_MODE->6-PWM-Mode (GND)
    GPIO_setPinConfig(GPIO_42_GPIO42);
    GPIO_writePin(42, 0);
    GPIO_setDirectionMode(42, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(42, GPIO_PIN_TYPE_STD);

    // GPIO43->Reserve
    GPIO_setPinConfig(GPIO_43_GPIO43);
    GPIO_setDirectionMode(43, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(43, GPIO_PIN_TYPE_STD);

    // GPIO44->Reserve
    GPIO_setPinConfig(GPIO_44_GPIO44);
    GPIO_setDirectionMode(44, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(44, GPIO_PIN_TYPE_STD);

    // GPIO45->M2_DRV_MODE->6-PWM-Mode (GND)
    GPIO_setPinConfig(GPIO_45_GPIO45);
    GPIO_writePin(45, 0);
    GPIO_setDirectionMode(45, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(45, GPIO_PIN_TYPE_STD);

    // GPIO46->M2_DRV_CAL->Low
    GPIO_setPinConfig(GPIO_46_GPIO46);
    GPIO_writePin(46, 0);
    GPIO_setDirectionMode(46, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(46, GPIO_PIN_TYPE_STD);

    // GPIO47->Reserve
    GPIO_setPinConfig(GPIO_47_GPIO47);
    GPIO_setDirectionMode(47, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(47, GPIO_PIN_TYPE_STD);

    // GPIO48->M1_LED
    GPIO_setPinConfig(GPIO_48_GPIO48);
    GPIO_writePin(48, 0);
    GPIO_setDirectionMode(48, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(48, GPIO_PIN_TYPE_STD);

    // GPIO51->Reserve
    GPIO_setPinConfig(GPIO_51_GPIO51);
    GPIO_setDirectionMode(51, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(51, GPIO_PIN_TYPE_STD);

    // GPIO55->HALL_V
    GPIO_setPinConfig(GPIO_55_GPIO55);
    GPIO_setDirectionMode(55, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(55, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(55, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(55, 2);

#if defined(SFRA_ENABLE)
    // GPIO56->SCIA_RX for SFRA
    GPIO_setPinConfig(GPIO_56_SCIB_TX);
    GPIO_setDirectionMode(56, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(56, GPIO_PIN_TYPE_STD);
#else  // !SFRA_ENABLE
    // GPIO56->M2_DRV_ENABLE
    GPIO_setPinConfig(GPIO_56_GPIO56);
    GPIO_writePin(56, 1);
    GPIO_setDirectionMode(56, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(56, GPIO_PIN_TYPE_STD);
#endif

    // GPIO57->HALL_W
    GPIO_setPinConfig(GPIO_57_GPIO57);
    GPIO_setDirectionMode(57, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(57, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(57, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(57, 2);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO58->SPIB_CLK->DAC128S_SCLK
    GPIO_setPinConfig(GPIO_58_SPIB_CLK);
    GPIO_setDirectionMode(58, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(58, GPIO_PIN_TYPE_STD);
#else  // !DACEXT_ENABLE
    // GPIO58->Reserve GPIO
    GPIO_setPinConfig(GPIO_58_GPIO58);
    GPIO_setDirectionMode(58, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(58, GPIO_PIN_TYPE_STD);
#endif  // !DACEXT_ENABLE

    // GPIO59->QEP1_INDEX (J12)
    GPIO_setPinConfig(GPIO_59_EQEP1_INDEX);
    GPIO_setDirectionMode(59, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(59, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(59, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(59, 2);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO60->SPIB_SIMO->DAC128S_SDI
    GPIO_setPinConfig(GPIO_60_SPIB_SIMO);
    GPIO_setDirectionMode(60, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(60, GPIO_PIN_TYPE_STD);
#else  // !DACEXT_ENABLE
    // GPIO60->Reserve
    GPIO_setPinConfig(GPIO_60_GPIO60);
    GPIO_setDirectionMode(60, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(60, GPIO_PIN_TYPE_STD);
#endif  // !DACEXT_ENABLE

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO61->SPIB_SOMI->DAC128S_SDO
    GPIO_setPinConfig(GPIO_61_SPIB_SOMI);
    GPIO_setDirectionMode(61, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(61, GPIO_PIN_TYPE_STD);
#else  // !DACEXT_ENABLE
    // GPIO61->Reserve
    GPIO_setPinConfig(GPIO_61_GPIO61);
    GPIO_setDirectionMode(61, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(61, GPIO_PIN_TYPE_STD);
#endif  // !DACEXT_ENABLE

    // end ofBSXL8323RS_REVA
//------------------------------------------------------------------------------
#elif defined(BSXL8323RH_REVB)

    // GPIO0->EPWM1A->M1_UH*
    GPIO_setPinConfig(GPIO_0_EPWM1_A);
    GPIO_setDirectionMode(0, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(0, GPIO_PIN_TYPE_STD);

    // GPIO1->EPWM1B->M1_UL*
    GPIO_setPinConfig(GPIO_1_EPWM1_B);
    GPIO_setDirectionMode(1, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(1, GPIO_PIN_TYPE_STD);
    // GPIO2->EPWM2A->M1_VH*
    GPIO_setPinConfig(GPIO_2_EPWM2_A);
    GPIO_setDirectionMode(2, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(2, GPIO_PIN_TYPE_STD);

    // GPIO3->EPWM2B->M1_VL*
    GPIO_setPinConfig(GPIO_3_EPWM2_B);
    GPIO_setDirectionMode(3, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(3, GPIO_PIN_TYPE_STD);

#if defined(CMD_CAN_EN)
    // GPIO4->CAN_TX
    GPIO_setPinConfig(GPIO_4_CANA_TX);
    GPIO_setDirectionMode(4, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_STD);
#else   // !CMD_CAN_EN
    // GPIO4->M2_nFAULT
    GPIO_setPinConfig(GPIO_4_GPIO4);
    GPIO_setDirectionMode(4, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_PULLUP);
#endif  // !CMD_CAN_EN

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIA)
    // GPIO5->DAC128S_SYNC->Use a Jumper connect to J2-12
    GPIO_setPinConfig(GPIO_5_SPIA_STE);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);
#elif defined(CMD_CAN_EN)
    // GPIO5->CAN_RX
    GPIO_setPinConfig(GPIO_5_CANA_RX);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);
#else   // !CMD_CAN_EN
    // GPIO5->Reserve GPIO
    GPIO_setPinConfig(GPIO_5_GPIO5);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);
#endif  // !(DAC128S_ENABLE && DAC128S_SPIA) & !CMD_CAN_EN

    // GPIO6->Reserve
    GPIO_setPinConfig(GPIO_6_GPIO6);
    GPIO_setDirectionMode(6, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(6, GPIO_PIN_TYPE_STD);

    // GPIO7->Reserve
    GPIO_setPinConfig(GPIO_7_GPIO7);
    GPIO_setDirectionMode(7, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(7, GPIO_PIN_TYPE_STD);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIA)
    // GPIO8->SPIA_SIMO->DAC12S_SDI
    GPIO_setPinConfig(GPIO_8_SPIA_SIMO);
    GPIO_setDirectionMode(8, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(8, GPIO_PIN_TYPE_STD);
#else
    // GPIO8->Reserve
    GPIO_setPinConfig(GPIO_8_GPIO8);
    GPIO_setDirectionMode(8, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(8, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIA

    // GPIO9->Reserve GPIO
    GPIO_setPinConfig(GPIO_9_GPIO9);
    GPIO_setDirectionMode(9, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(9, GPIO_PIN_TYPE_STD);

    // GPIO10->EPWM6A->M1_WH*
    GPIO_setPinConfig(GPIO_10_EPWM6_A);
    GPIO_setDirectionMode(10, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(10, GPIO_PIN_TYPE_STD);

    // GPIO11->EPWM6B->M1_WL*
    GPIO_setPinConfig(GPIO_11_EPWM6_B);
    GPIO_setDirectionMode(11, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(11, GPIO_PIN_TYPE_STD);

    // GPIO12->Reserve
    GPIO_setPinConfig(GPIO_12_GPIO12);
    GPIO_setDirectionMode(12, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(12, GPIO_PIN_TYPE_STD);

    // GPIO13->Reserve
    GPIO_setPinConfig(GPIO_13_GPIO13);
    GPIO_setDirectionMode(13, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(13, GPIO_PIN_TYPE_STD);

    // GPIO14->HALL_U
    GPIO_setPinConfig(GPIO_14_GPIO14);
    GPIO_setDirectionMode(14, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(14, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(14, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(14, 2);

#if defined(SFRA_ENABLE)
    // GPIO15->SCIA_RX for SFRA
    GPIO_setPinConfig(GPIO_15_SCIB_RX);
    GPIO_setDirectionMode(15, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(15, GPIO_PIN_TYPE_STD);
#else  // !SFRA_ENABLE
    // GPIO15->Reserve
    GPIO_setPinConfig(GPIO_15_GPIO15);
    GPIO_setDirectionMode(15, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(15, GPIO_PIN_TYPE_STD);
#endif

    // GPIO16->EPWM5A->M2_WH*
    GPIO_setPinConfig(GPIO_16_EPWM5_A);
    GPIO_setDirectionMode(16, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(16, GPIO_PIN_TYPE_STD);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIA)
    // GPIO17->SPIA_SOMI->DAC12S_SDO
    GPIO_setPinConfig(GPIO_17_SPIA_SOMI);
    GPIO_setDirectionMode(17, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(17, GPIO_PIN_TYPE_STD);
#else
    // GPIO17->Reserve
    GPIO_setPinConfig(GPIO_17_GPIO17);
    GPIO_setDirectionMode(17, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(17, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIA

    // GPIO18->Reserve
    GPIO_setPinConfig(GPIO_18_GPIO18);
    GPIO_setDirectionMode(18, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(18, GPIO_PIN_TYPE_STD);

    // GPIO19->Reserve
    GPIO_setPinConfig(GPIO_19_GPIO19);
    GPIO_setDirectionMode(19, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(19, GPIO_PIN_TYPE_STD);

    // GPIO20->LP_LED-RED
    GPIO_setAnalogMode(20, GPIO_ANALOG_DISABLED);
    GPIO_setPinConfig(GPIO_20_GPIO20);
    GPIO_writePin(20, 0);
    GPIO_setDirectionMode(20, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(20, GPIO_PIN_TYPE_STD);

    // GPIO21->M2_DRV_MODE->6-PWM-Mode (->GND)
    GPIO_setAnalogMode(21, GPIO_ANALOG_DISABLED);
    GPIO_setPinConfig(GPIO_21_GPIO21);
    GPIO_writePin(21, 0);
    GPIO_setDirectionMode(21, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(21, GPIO_PIN_TYPE_STD);

    // GPIO22->M1_DRV_nSCS/GAIN (has a 47k pull down)
    GPIO_setPinConfig(GPIO_22_GPIO22);
    GPIO_writePin(20, 0);
    GPIO_setDirectionMode(22, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(22, GPIO_PIN_TYPE_STD);

    // GPIO23->M1_DRV_CAL (Low)
    GPIO_setPinConfig(GPIO_23_GPIO23);
    GPIO_writePin(23, 0);
    GPIO_setDirectionMode(23, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(23, GPIO_PIN_TYPE_STD);

    // GPIO24->M1_DRV_nFAULT*
    GPIO_setPinConfig(GPIO_24_GPIO24);
    GPIO_setDirectionMode(24, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(24, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(24, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(24, 2);

    // GPIO25->Reserve
    GPIO_setPinConfig(GPIO_25_GPIO25);
    GPIO_setDirectionMode(25, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(25, GPIO_PIN_TYPE_STD);

    // GPIO26->Reserve
    GPIO_setPinConfig(GPIO_26_GPIO26);
    GPIO_setDirectionMode(26, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(26, GPIO_PIN_TYPE_STD);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO27->SPIB_STE->DAC128S_SYNC
    GPIO_setPinConfig(GPIO_27_SPIB_STE);
    GPIO_setDirectionMode(27, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(27, GPIO_PIN_TYPE_STD);
#else
    // GPIO27->Reserve
    GPIO_setPinConfig(GPIO_27_GPIO27);
    GPIO_setDirectionMode(27, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(27, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIB

    // GPIO28->HAL_GPIO_ISR_M1
    GPIO_setPinConfig(GPIO_28_GPIO28);
    GPIO_writePin(28, 1);
    GPIO_setDirectionMode(28, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(28, GPIO_PIN_TYPE_PULLUP);

    // GPIO29->M1_DRV_ENABLE*
    GPIO_setPinConfig(GPIO_29_GPIO29);
    GPIO_writePin(29, 1);
    GPIO_setDirectionMode(29, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(29, GPIO_PIN_TYPE_PULLUP);

    // GPIO30->Reserve
    GPIO_setPinConfig(GPIO_30_GPIO30);
    GPIO_setDirectionMode(30, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(30, GPIO_PIN_TYPE_STD);

    // GPIO31->M2_LED
    GPIO_setPinConfig(GPIO_31_GPIO31);
    GPIO_writePin(31, 0);
    GPIO_setDirectionMode(31, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(31, GPIO_PIN_TYPE_STD);

    // GPIO32->Reserve
    GPIO_setPinConfig(GPIO_32_GPIO32);
    GPIO_setDirectionMode(32, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(32, GPIO_PIN_TYPE_STD);

    // GPIO33->M1_DRV_MODE->6-PWM-Mode (->GND)
    GPIO_setPinConfig(GPIO_33_GPIO33);
    GPIO_writePin(33, 0);
    GPIO_setDirectionMode(33, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(33, GPIO_PIN_TYPE_STD);

    // GPIO34->Reserve
    GPIO_setPinConfig(GPIO_34_GPIO34);
    GPIO_setDirectionMode(34, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(34, GPIO_PIN_TYPE_STD);

#if !defined(MOTOR2_ISBLDC)
    // GPIO35->EPWM5B->M2_WL
    GPIO_setPinConfig(GPIO_35_EPWM5_B);
    GPIO_setDirectionMode(35, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(35, GPIO_PIN_TYPE_STD);
#else
    GPIO_setPinConfig(GPIO_35_GPIO35);
    GPIO_setDirectionMode(35, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(35, GPIO_PIN_TYPE_STD);
#endif // !MOTOR2_ISBLDC

    // GPIO37->Reserve
    GPIO_setPinConfig(GPIO_37_GPIO37);
    GPIO_setDirectionMode(37, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(37, GPIO_PIN_TYPE_STD);

    // GPIO39->HAL_GPIO_ISR_M1
    GPIO_setPinConfig(GPIO_39_GPIO39);
    GPIO_setDirectionMode(39, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(39, GPIO_PIN_TYPE_STD);

    // GPIO40->QEP1_A
    GPIO_setPinConfig(GPIO_40_EQEP1_A);
    GPIO_setDirectionMode(40, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(40, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(40, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(40, 2);

    // GPIO41->QEP1_B
    GPIO_setPinConfig(GPIO_41_EQEP1_B);
    GPIO_setDirectionMode(41, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(41, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(41, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(41, 2);

    // GPIO42->M2_DRV_MODE->6-PWM-Mode (GND)
    GPIO_setPinConfig(GPIO_42_GPIO42);
    GPIO_writePin(42, 0);
    GPIO_setDirectionMode(42, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(42, GPIO_PIN_TYPE_STD);

    // GPIO43->Reserve
    GPIO_setPinConfig(GPIO_43_GPIO43);
    GPIO_setDirectionMode(43, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(43, GPIO_PIN_TYPE_STD);

    // GPIO44->Reserve
    GPIO_setPinConfig(GPIO_44_GPIO44);
    GPIO_setDirectionMode(44, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(44, GPIO_PIN_TYPE_STD);

    // GPIO45->M2_DRV_MODE->6-PWM-Mode (GND)
    GPIO_setPinConfig(GPIO_45_GPIO45);
    GPIO_writePin(45, 0);
    GPIO_setDirectionMode(45, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(45, GPIO_PIN_TYPE_STD);

    // GPIO46->M2_DRV_CAL->Low
    GPIO_setPinConfig(GPIO_46_GPIO46);
    GPIO_writePin(46, 0);
    GPIO_setDirectionMode(46, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(46, GPIO_PIN_TYPE_STD);

    // GPIO47->Reserve
    GPIO_setPinConfig(GPIO_47_GPIO47);
    GPIO_setDirectionMode(47, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(47, GPIO_PIN_TYPE_STD);

    // GPIO48->M1_LED
    GPIO_setPinConfig(GPIO_48_GPIO48);
    GPIO_writePin(48, 0);
    GPIO_setDirectionMode(48, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(48, GPIO_PIN_TYPE_STD);

    // GPIO51->Reserve
    GPIO_setPinConfig(GPIO_51_GPIO51);
    GPIO_setDirectionMode(51, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(51, GPIO_PIN_TYPE_STD);

    // GPIO55->HALL_V
    GPIO_setPinConfig(GPIO_55_GPIO55);
    GPIO_setDirectionMode(55, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(55, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(55, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(55, 2);

#if defined(SFRA_ENABLE)
    // GPIO56->SCIA_RX for SFRA
    GPIO_setPinConfig(GPIO_56_SCIB_TX);
    GPIO_setDirectionMode(56, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(56, GPIO_PIN_TYPE_STD);
#else  // !SFRA_ENABLE
    // GPIO56->M2_DRV_ENABLE
    GPIO_setPinConfig(GPIO_56_GPIO56);
    GPIO_writePin(56, 1);
    GPIO_setDirectionMode(56, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(56, GPIO_PIN_TYPE_STD);
#endif

    // GPIO57->HALL_W
    GPIO_setPinConfig(GPIO_57_GPIO57);
    GPIO_setDirectionMode(57, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(57, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(57, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(57, 2);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO58->SPIB_CLK->DAC128S_SCLK
    GPIO_setPinConfig(GPIO_58_SPIB_CLK);
    GPIO_setDirectionMode(58, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(58, GPIO_PIN_TYPE_STD);
#else
    // GPIO58->Reserve GPIO
    GPIO_setPinConfig(GPIO_58_GPIO58);
    GPIO_setDirectionMode(58, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(58, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIB

    // GPIO59->QEP1_INDEX (J12)
    GPIO_setPinConfig(GPIO_59_EQEP1_INDEX);
    GPIO_setDirectionMode(59, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(59, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(59, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(59, 2);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO60->SPIB_SIMO->DAC128S_SDI
    GPIO_setPinConfig(GPIO_60_SPIB_SIMO);
    GPIO_setDirectionMode(60, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(60, GPIO_PIN_TYPE_STD);
#else   // DAC128S_ENABLE && DAC128S_SPIB
    // GPIO60->Reserve
    GPIO_setPinConfig(GPIO_60_GPIO60);
    GPIO_setDirectionMode(60, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(60, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIB

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO61->SPIB_SOMI->DAC128S_SDO
    GPIO_setPinConfig(GPIO_61_SPIB_SOMI);
    GPIO_setDirectionMode(61, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(61, GPIO_PIN_TYPE_STD);
#else   // DAC128S_ENABLE && DAC128S_SPIB
    // GPIO61->Reserve
    GPIO_setPinConfig(GPIO_61_GPIO61);
    GPIO_setDirectionMode(61, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(61, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIB

    // end ofBSXL8323RH_REVB
//------------------------------------------------------------------------------
#elif defined(BSXL8353RS_REVA)
    // GPIO0->EPWM1A->M1_UH*
    GPIO_setPinConfig(GPIO_0_EPWM1_A);
    GPIO_setDirectionMode(0, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(0, GPIO_PIN_TYPE_STD);

    // GPIO1->EPWM1B->M1_UL*
    GPIO_setPinConfig(GPIO_1_EPWM1_B);
    GPIO_setDirectionMode(1, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(1, GPIO_PIN_TYPE_STD);

    // GPIO2->EPWM2A->M1_VH*
    GPIO_setPinConfig(GPIO_2_EPWM2_A);
    GPIO_setDirectionMode(2, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(2, GPIO_PIN_TYPE_STD);

    // GPIO3->EPWM2B->M1_VL*
    GPIO_setPinConfig(GPIO_3_EPWM2_B);
    GPIO_setDirectionMode(3, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(3, GPIO_PIN_TYPE_STD);

#if defined(CMD_CAN_EN)
    // GPIO4->CAN_TX
    GPIO_setPinConfig(GPIO_4_CANA_TX);
    GPIO_setDirectionMode(4, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_STD);
#else   // !CMD_CAN_EN
    // GPIO4->M2_nFAULT
    GPIO_setPinConfig(GPIO_4_GPIO4);
    GPIO_setDirectionMode(4, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_PULLUP);
#endif  // !CMD_CAN_EN

#if defined(CMD_CAN_EN)
    // GPIO5->CAN_RX
    GPIO_setPinConfig(GPIO_5_CANA_RX);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);
#elif !defined(DRV_CS_GPIO) || defined(DAC128S_SPIA)
    // GPIO5->M1_DRV_SCS->Use a Jumper connect to J2-12
    GPIO_setPinConfig(GPIO_5_SPIA_STE);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_PULLUP);
#else
    // GPIO5->Reserve
    GPIO_setPinConfig(GPIO_5_GPIO5);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);
#endif  // CMD_CAN_EN)

    // GPIO6->Reserve
    GPIO_setPinConfig(GPIO_6_GPIO6);
    GPIO_setDirectionMode(6, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(6, GPIO_PIN_TYPE_STD);

    // GPIO7->Reserve
    GPIO_setPinConfig(GPIO_7_GPIO7);
    GPIO_setDirectionMode(7, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(7, GPIO_PIN_TYPE_STD);

    // GPIO8->SPIA_SIMO->M1_DRV_SDI or DAC12S_SDI
    GPIO_setPinConfig(GPIO_8_SPIA_SIMO);
    GPIO_setDirectionMode(8, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(8, GPIO_PIN_TYPE_STD);

    // GPIO9->SPIA_CLK->M1_DRV_SCLK or DAC128S_SCLK
    GPIO_setPinConfig(GPIO_9_SPIA_CLK);
    GPIO_setDirectionMode(9, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(9, GPIO_PIN_TYPE_STD);

    // GPIO10->EPWM6A->M1_WH*
    GPIO_setPinConfig(GPIO_10_EPWM6_A);
    GPIO_setDirectionMode(10, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(10, GPIO_PIN_TYPE_STD);

    // GPIO11->EPWM6B->M1_WL*
    GPIO_setPinConfig(GPIO_11_EPWM6_B);
    GPIO_setDirectionMode(11, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(11, GPIO_PIN_TYPE_STD);

    // GPIO12->Reserve /// pwm// -> can B
   // GPIO_setPinConfig(GPIO_12_EPWM7_A);
    //GPIO_setDirectionMode(12, GPIO_DIR_MODE_OUT);
    //GPIO_setPadConfig(12, GPIO_PIN_TYPE_STD);

    // GPIO13->Reserve
   //GPIO_setPinConfig(GPIO_13_GPIO13);
   // GPIO_setDirectionMode(13, GPIO_DIR_MODE_OUT);
   // GPIO_setPadConfig(13, GPIO_PIN_TYPE_STD);

    // GPIO14->HALL_U
    GPIO_setPinConfig(GPIO_14_GPIO14);
    GPIO_setDirectionMode(14, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(14, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(14, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(14, 2);

#if defined(SFRA_ENABLE)
    // GPIO15->SCIA_RX for SFRA
    GPIO_setPinConfig(GPIO_15_SCIB_RX);
    GPIO_setDirectionMode(15, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(15, GPIO_PIN_TYPE_STD);
#else  // !SFRA_ENABLE
    // GPIO15->Reserve
    GPIO_setPinConfig(GPIO_15_GPIO15);
    GPIO_setDirectionMode(15, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(15, GPIO_PIN_TYPE_STD);
#endif

    // GPIO16->EPWM5A->M2_WH*
    GPIO_setPinConfig(GPIO_16_EPWM5_A);
    GPIO_setDirectionMode(16, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(16, GPIO_PIN_TYPE_STD);

    // GPIO17->SPIA_SOMI->M1_DRV_SDO or DAC12S_SDO
    GPIO_setPinConfig(GPIO_17_SPIA_SOMI);
    GPIO_setDirectionMode(17, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(17, GPIO_PIN_TYPE_STD);

    // GPIO18->Reserve
    GPIO_setPinConfig(GPIO_18_GPIO18);
    GPIO_setDirectionMode(18, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(18, GPIO_PIN_TYPE_STD);

    // GPIO19->Reserve
    GPIO_setPinConfig(GPIO_19_GPIO19);
    GPIO_setDirectionMode(19, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(19, GPIO_PIN_TYPE_STD);

    // GPIO20->LP_LED-RED
    GPIO_setAnalogMode(20, GPIO_ANALOG_DISABLED);
    GPIO_setPinConfig(GPIO_20_GPIO20);
    GPIO_writePin(20, 0);
    GPIO_setDirectionMode(20, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(20, GPIO_PIN_TYPE_STD);

    // GPIO21->M2_DRV_MODE->6-PWM-Mode (->GND)
    GPIO_setAnalogMode(21, GPIO_ANALOG_DISABLED);
    GPIO_setPinConfig(GPIO_21_GPIO21);
    GPIO_writePin(21, 0);
    GPIO_setDirectionMode(21, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(21, GPIO_PIN_TYPE_STD);

    // GPIO22->M1_DRV_nSCS(GPIO Mode)
#if defined(DRV_CS_GPIO)
    GPIO_setPinConfig(GPIO_22_GPIO22);
    GPIO_writePin(22, 1);
    GPIO_setDirectionMode(22, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(22, GPIO_PIN_TYPE_STD);
#else
    GPIO_setPinConfig(GPIO_22_GPIO22);
    GPIO_setDirectionMode(22, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(22, GPIO_PIN_TYPE_STD);
#endif

    // GPIO23->M1_DRV_CAL (Low)
    GPIO_setPinConfig(GPIO_23_GPIO23);
    GPIO_writePin(23, 0);
    GPIO_setDirectionMode(23, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(23, GPIO_PIN_TYPE_STD);

    // GPIO24->M1_DRV_nFAULT*
    GPIO_setPinConfig(GPIO_24_GPIO24);
    GPIO_setDirectionMode(24, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(24, GPIO_PIN_TYPE_PULLUP);

    // GPIO25->Reserve
    GPIO_setPinConfig(GPIO_25_GPIO25);
    GPIO_setDirectionMode(25, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(25, GPIO_PIN_TYPE_STD);

    // GPIO26->M2_DRV_SCS(GPIO Mode)
    GPIO_setPinConfig(GPIO_26_GPIO26);
    GPIO_writePin(26, 1);
    GPIO_setDirectionMode(26, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(26, GPIO_PIN_TYPE_STD);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO27->SPIB_STE->DAC128S_SYNC
    GPIO_setPinConfig(GPIO_27_SPIB_STE);
    GPIO_setDirectionMode(27, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(27, GPIO_PIN_TYPE_STD);
#else
    // GPIO27->Reserve
    GPIO_setPinConfig(GPIO_27_GPIO27);
    GPIO_setDirectionMode(27, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(27, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIB

    // GPIO28->HAL_GPIO_ISR_M1
    GPIO_setPinConfig(GPIO_28_GPIO28);
    GPIO_writePin(28, 1);
    GPIO_setDirectionMode(28, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(28, GPIO_PIN_TYPE_PULLUP);

    // GPIO29->M1_DRV_ENABLE*
    GPIO_setPinConfig(GPIO_29_GPIO29);
    GPIO_writePin(29, 1);
    GPIO_setDirectionMode(29, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(29, GPIO_PIN_TYPE_PULLUP);

    // GPIO30->Reserve
    GPIO_setPinConfig(GPIO_30_GPIO30);
    GPIO_setDirectionMode(30, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(30, GPIO_PIN_TYPE_STD);

    // GPIO31->M2_LED
    GPIO_setPinConfig(GPIO_31_GPIO31);
    GPIO_writePin(31, 0);
    GPIO_setDirectionMode(31, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(31, GPIO_PIN_TYPE_STD);

    // GPIO32->Reserve
    GPIO_setPinConfig(GPIO_32_GPIO32);
    GPIO_setDirectionMode(32, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(32, GPIO_PIN_TYPE_STD);

    // GPIO33->M1_DRV_MODE->6-PWM-Mode (->GND)
    GPIO_setPinConfig(GPIO_33_GPIO33);
    GPIO_writePin(33, 0);
    GPIO_setDirectionMode(33, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(33, GPIO_PIN_TYPE_STD);

    // GPIO34->Reserve
    GPIO_setPinConfig(GPIO_34_GPIO34);
    GPIO_setDirectionMode(34, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(34, GPIO_PIN_TYPE_STD);

#if !defined(MOTOR2_ISBLDC)
    // GPIO35->EPWM5B->M2_WL
    GPIO_setPinConfig(GPIO_35_EPWM5_B);
    GPIO_setDirectionMode(35, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(35, GPIO_PIN_TYPE_STD);
#else
    GPIO_setPinConfig(GPIO_35_GPIO35);
    GPIO_setDirectionMode(35, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(35, GPIO_PIN_TYPE_STD);
#endif // !MOTOR2_ISBLDC

    // GPIO37->Reserve
    GPIO_setPinConfig(GPIO_37_GPIO37);
    GPIO_setDirectionMode(37, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(37, GPIO_PIN_TYPE_STD);

    // GPIO39->HAL_GPIO_ISR_M1
    GPIO_setPinConfig(GPIO_39_GPIO39);
    GPIO_setDirectionMode(39, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(39, GPIO_PIN_TYPE_STD);

    // GPIO40->QEP1_A (J12)
    GPIO_setPinConfig(GPIO_40_EQEP1_A);
    GPIO_setDirectionMode(40, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(40, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(40, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(40, 2);

    // GPIO41->QEP1_B (J12)
    GPIO_setPinConfig(GPIO_41_EQEP1_B);
    GPIO_setDirectionMode(41, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(41, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(41, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(41, 2);

    // GPIO42->M1_DRV_MODE->6-PWM-Mode (GND)
    GPIO_setPinConfig(GPIO_42_GPIO42);
    GPIO_writePin(42, 0);
    GPIO_setDirectionMode(42, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(42, GPIO_PIN_TYPE_STD);

    // GPIO43->Reserve
    GPIO_setPinConfig(GPIO_43_GPIO43);
    GPIO_setDirectionMode(43, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(43, GPIO_PIN_TYPE_STD);

    // GPIO44->Reserve
    GPIO_setPinConfig(GPIO_44_GPIO44);
    GPIO_setDirectionMode(44, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(44, GPIO_PIN_TYPE_STD);

    // GPIO45->M2_DRV_MODE->6-PWM-Mode (GND)
    GPIO_setPinConfig(GPIO_45_GPIO45);
    GPIO_writePin(45, 0);
    GPIO_setDirectionMode(45, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(45, GPIO_PIN_TYPE_STD);

    // GPIO46->M2_DRV_CAL->Low
    GPIO_setPinConfig(GPIO_46_GPIO46);
    GPIO_writePin(46, 0);
    GPIO_setDirectionMode(46, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(46, GPIO_PIN_TYPE_STD);

    // GPIO47->Reserve
    GPIO_setPinConfig(GPIO_47_GPIO47);
    GPIO_setDirectionMode(47, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(47, GPIO_PIN_TYPE_STD);

    // GPIO48->M1_LED
    GPIO_setPinConfig(GPIO_48_GPIO48);
    GPIO_writePin(48, 0);
    GPIO_setDirectionMode(48, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(48, GPIO_PIN_TYPE_STD);

    // GPIO51->Reserve
    GPIO_setPinConfig(GPIO_51_GPIO51);
    GPIO_setDirectionMode(51, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(51, GPIO_PIN_TYPE_STD);

    // GPIO55->HALL_V
    GPIO_setPinConfig(GPIO_55_GPIO55);
    GPIO_setDirectionMode(55, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(55, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(55, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(55, 2);

#if defined(SFRA_ENABLE)
    // GPIO56->SCIA_RX for SFRA
    GPIO_setPinConfig(GPIO_56_SCIB_TX);
    GPIO_setDirectionMode(56, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(56, GPIO_PIN_TYPE_STD);
#else  // !SFRA_ENABLE
    // GPIO56->M2_DRV_ENABLE
    GPIO_setPinConfig(GPIO_56_GPIO56);
    GPIO_writePin(56, 1);
    GPIO_setDirectionMode(56, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(56, GPIO_PIN_TYPE_STD);
#endif

    // GPIO57->HALL_W
    GPIO_setPinConfig(GPIO_57_GPIO57);
    GPIO_setDirectionMode(57, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(57, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(57, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(57, 2);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO58->SPIB_CLK->DAC128S_SCLK
    GPIO_setPinConfig(GPIO_58_SPIB_CLK);
    GPIO_setDirectionMode(58, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(58, GPIO_PIN_TYPE_STD);
#else
    // GPIO58->Reserve GPIO
    GPIO_setPinConfig(GPIO_58_GPIO58);
    GPIO_setDirectionMode(58, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(58, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIB

    // GPIO59->QEP1_INDEX (J12)
    GPIO_setPinConfig(GPIO_59_EQEP1_INDEX);
    GPIO_setDirectionMode(59, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(59, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(59, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(59, 2);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO60->SPIB_SIMO->DAC128S_SDI
    GPIO_setPinConfig(GPIO_60_SPIB_SIMO);
    GPIO_setDirectionMode(60, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(60, GPIO_PIN_TYPE_STD);
#else   // DAC128S_ENABLE && DAC128S_SPIB
    // GPIO60->Reserve
    GPIO_setPinConfig(GPIO_60_GPIO60);
    GPIO_setDirectionMode(60, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(60, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIB

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO61->SPIB_SOMI->DAC128S_SDO
    GPIO_setPinConfig(GPIO_61_SPIB_SOMI);
    GPIO_setDirectionMode(61, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(61, GPIO_PIN_TYPE_STD);
#else   // DAC128S_ENABLE && DAC128S_SPIB
    // GPIO61->Reserve
    GPIO_setPinConfig(GPIO_61_GPIO61);
    GPIO_setDirectionMode(61, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(61, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIB

    // end of BSXL8353RS_REVA
//-----------------------------------------------------------------------------
#elif defined(BSXL8316RT_REVA)
    // GPIO0->EPWM1A->M1_UH*
    GPIO_setPinConfig(GPIO_0_EPWM1_A);
    GPIO_setDirectionMode(0, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(0, GPIO_PIN_TYPE_STD);

    // GPIO1->EPWM1B->M1_UL*
    GPIO_setPinConfig(GPIO_1_EPWM1_B);
    GPIO_setDirectionMode(1, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(1, GPIO_PIN_TYPE_STD);

    // GPIO2->EPWM2A->M1_VH*
    GPIO_setPinConfig(GPIO_2_EPWM2_A);
    GPIO_setDirectionMode(2, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(2, GPIO_PIN_TYPE_STD);

    // GPIO3->EPWM2B->M1_VL*
    GPIO_setPinConfig(GPIO_3_EPWM2_B);
    GPIO_setDirectionMode(3, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(3, GPIO_PIN_TYPE_STD);

#if defined(CMD_CAN_EN)
    // GPIO4->CAN_TX
    GPIO_setPinConfig(GPIO_4_CANA_TX);
    GPIO_setDirectionMode(4, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_STD);
#else   // !CMD_CAN_EN
    // GPIO4->M2_nFAULT
    GPIO_setPinConfig(GPIO_4_GPIO4);
    GPIO_setDirectionMode(4, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_PULLUP);
#endif  // !CMD_CAN_EN

#if defined(CMD_CAN_EN)
    // GPIO5->CAN_RX
    GPIO_setPinConfig(GPIO_5_CANA_RX);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);
//#else
    // GPIO5->M1_DRV_SCS
    //GPIO_setPinConfig(GPIO_5_SPIA_STE);
    GPIO_setPinConfig(GPIO_26_GPIO26);
    GPIO_setDirectionMode(26, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(26, GPIO_PIN_TYPE_STD);
#endif  // CMD_CAN_EN)

    // GPIO6->Reserve
    GPIO_setPinConfig(GPIO_6_GPIO6);
    GPIO_setDirectionMode(6, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(6, GPIO_PIN_TYPE_STD);

    // GPIO7->Reserve
    GPIO_setPinConfig(GPIO_7_GPIO7);
    GPIO_setDirectionMode(7, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(7, GPIO_PIN_TYPE_STD);

    // GPIO8->SPIA_SIMO->M1_DRV_SDI or DAC12S_SDI
    GPIO_setPinConfig(GPIO_8_SPIA_SIMO);
    GPIO_setDirectionMode(8, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(8, GPIO_PIN_TYPE_STD);

    // GPIO9->SPIA_CLK->M1_DRV_SCLK or DAC128S_SCLK
    GPIO_setPinConfig(GPIO_9_SPIA_CLK);
    GPIO_setDirectionMode(9, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(9, GPIO_PIN_TYPE_STD);

    // GPIO10->EPWM6A->M1_WH*
    GPIO_setPinConfig(GPIO_10_EPWM6_A);
    GPIO_setDirectionMode(10, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(10, GPIO_PIN_TYPE_STD);

    // GPIO11->EPWM6B->M1_WL*
    GPIO_setPinConfig(GPIO_11_EPWM6_B);
    GPIO_setDirectionMode(11, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(11, GPIO_PIN_TYPE_STD);

    // GPIO12->Reserve
   // GPIO_setPinConfig(GPIO_12_GPIO12);
   // GPIO_setDirectionMode(12, GPIO_DIR_MODE_IN);
   // GPIO_setPadConfig(12, GPIO_PIN_TYPE_STD);

    GPIO_setPinConfig(GPIO_12_GPIO12);
    GPIO_setDirectionMode(12, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(12, GPIO_PIN_TYPE_PULLUP);
    GPIO_writePin(12, 0);

    // GPIO13->Reserve
    GPIO_setPinConfig(GPIO_13_GPIO13);
    GPIO_setDirectionMode(13, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(13, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(13, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(13, 2);

    // GPIO14->HALL_U
    GPIO_setPinConfig(GPIO_14_GPIO14);
    GPIO_setDirectionMode(14, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(14, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(14, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(14, 2);

#if defined(SFRA_ENABLE)
    // GPIO15->SCIA_RX for SFRA
    GPIO_setPinConfig(GPIO_15_SCIB_RX);
    GPIO_setDirectionMode(15, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(15, GPIO_PIN_TYPE_STD);
#else  // !SFRA_ENABLE
    // GPIO15->Reserve
    GPIO_setPinConfig(GPIO_15_GPIO15);
    GPIO_setDirectionMode(15, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(15, GPIO_PIN_TYPE_STD);
#endif

    // GPIO16->Reserve
    GPIO_setPinConfig(GPIO_16_GPIO16);
    GPIO_setDirectionMode(16, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(16, GPIO_PIN_TYPE_PULLUP);

    // GPIO17->SPIA_SOMI->M1_DRV_SDO or DAC12S_SDO
    GPIO_setPinConfig(GPIO_17_SPIA_SOMI);
    GPIO_setDirectionMode(17, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(17, GPIO_PIN_TYPE_STD);

    // GPIO18->Reserve
    GPIO_setPinConfig(GPIO_18_GPIO18);
    GPIO_setDirectionMode(18, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(18, GPIO_PIN_TYPE_STD);

    // GPIO19->Reserve
    GPIO_setPinConfig(GPIO_19_GPIO19);
    GPIO_setDirectionMode(19, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(19, GPIO_PIN_TYPE_STD);

    // GPIO20->M2_DRV_OFF, 1- Disable, 0-Enable
    GPIO_setAnalogMode(44, GPIO_ANALOG_DISABLED);
    GPIO_setPinConfig(GPIO_44_GPIO44);
    GPIO_writePin(44, 0);
    GPIO_setDirectionMode(44, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(44, GPIO_PIN_TYPE_STD);

    // GPIO21->M2_DRV_MODE->6-PWM-Mode (->GND)
    GPIO_setAnalogMode(21, GPIO_ANALOG_DISABLED);
    GPIO_setPinConfig(GPIO_21_GPIO21);
    GPIO_writePin(21, 0);
    GPIO_setDirectionMode(21, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(21, GPIO_PIN_TYPE_STD);

    // GPIO22->Reserve
    GPIO_setPinConfig(GPIO_22_GPIO22);
    GPIO_setDirectionMode(22, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(22, GPIO_PIN_TYPE_STD);

    // GPIO23->M1_DRV_CAL (Low)
    GPIO_setPinConfig(GPIO_23_GPIO23);
    GPIO_writePin(23, 0);
    GPIO_setDirectionMode(23, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(23, GPIO_PIN_TYPE_STD);

    // GPIO24->M1_DRV_nFAULT*
    GPIO_setPinConfig(GPIO_41_GPIO41);
    GPIO_setDirectionMode(41, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(41, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(41, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(41, 2);

    // GPIO25->Reserve
    GPIO_setPinConfig(GPIO_25_GPIO25);
    GPIO_setDirectionMode(25, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(25, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(25, GPIO_QUAL_3SAMPLE);

     //GPIO26->M2_DRV_SCS(GPIO Mode)
     GPIO_setPinConfig(GPIO_26_GPIO26);
     GPIO_setDirectionMode(26, GPIO_DIR_MODE_OUT);
     GPIO_setPadConfig(26, GPIO_PIN_TYPE_PULLUP);
     GPIO_writePin(26, 0);

    // GPIO27->SPIB_STE->M2_DRV_SCS or DAC128S_SYNC  -> can spi cs rev. 251216
      
    GPIO_setPinConfig(GPIO_27_GPIO27);
    GPIO_setDirectionMode(27, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(27, GPIO_PIN_TYPE_PULLUP);
    GPIO_writePin(27, 0);
    
    // GPIO28->HAL_GPIO_ISR_M1
    GPIO_setPinConfig(GPIO_28_GPIO28);
    GPIO_writePin(28, 1);
    GPIO_setDirectionMode(28, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(28, GPIO_PIN_TYPE_PULLUP);

    // GPIO29->M1_DRV_ENABLE/M1_nSLEEP, 1-Active, 0-Low Power Sleep Mode
 //   GPIO_setPinConfig(GPIO_29_GPIO29);
 //   GPIO_writePin(29, 1);
 //   GPIO_setDirectionMode(29, GPIO_DIR_MODE_OUT);
 //   GPIO_setPadConfig(29, GPIO_PIN_TYPE_PULLUP);

    // GPIO30->Reserve
    GPIO_setPinConfig(GPIO_30_GPIO30);
    GPIO_setDirectionMode(30, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(30, GPIO_PIN_TYPE_STD);

    // GPIO31->Reserve
    GPIO_setPinConfig(GPIO_31_GPIO31);
    GPIO_setDirectionMode(31, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(31, GPIO_PIN_TYPE_STD);

    // GPIO32->Reserve
    GPIO_setPinConfig(GPIO_32_GPIO32);
    GPIO_setDirectionMode(32, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(32, GPIO_PIN_TYPE_STD);

    // HALL A
    GPIO_setPinConfig(GPIO_33_GPIO33);
    //GPIO_writePin(33, 0);
    GPIO_setDirectionMode(33, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(33, GPIO_PIN_TYPE_STD);

    // GPIO34->Reserve -> i2c
    /*GPIO_setPinConfig(GPIO_34_GPIO34);
    GPIO_setDirectionMode(34, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(34, GPIO_PIN_TYPE_STD);*/

    // GPIO35->EPWM5B->M2_WL
    GPIO_setPinConfig(GPIO_35_EPWM5_B);
    GPIO_setDirectionMode(35, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(35, GPIO_PIN_TYPE_STD);
    
     // GPIO_setPinConfig(GPIO_16_EPWM5_A);
    //GPIO_setDirectionMode(16, GPIO_DIR_MODE_OUT);
    //GPIO_setPadConfig(16, GPIO_PIN_TYPE_STD);


    
    // GPIO16->Reserve -> dac
    
  //  GPIO_setPinConfig(GPIO_16_GPIO16);
  //  GPIO_setDirectionMode(16, GPIO_DIR_MODE_IN);
  //  GPIO_setPadConfig(16, GPIO_PIN_TYPE_STD);
    // GPIO_setPinConfig(GPIO_16_GPIO16);
  //   GPIO_setPinConfig(GPIO_16_DACA_OUT);
   // GPIO_setAnalogMode(16, GPIO_ANALOG_ENABLED);


    // GPIO37->M1_DRV_OFF, 1- Disable, 0-Enable
 #if defined(evm)
    GPIO_setPinConfig(GPIO_37_GPIO37);
    GPIO_setDirectionMode(37, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(27, GPIO_PIN_TYPE_STD);
    GPIO_writePin(37, 0);
#else
    GPIO_setPinConfig(GPIO_44_GPIO44);
    GPIO_setDirectionMode(44, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(44, GPIO_PIN_TYPE_STD);
    GPIO_writePin(44, 0);
#endif
    // GPIO39->HAL_GPIO_ISR_M1
    GPIO_setPinConfig(GPIO_39_GPIO39);
    GPIO_setDirectionMode(39, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(39, GPIO_PIN_TYPE_STD);

    // nsleep
    GPIO_setPinConfig(GPIO_40_GPIO40);
    GPIO_setDirectionMode(40, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(40, GPIO_PIN_TYPE_STD);
 
   /* // GPIO40->QEP1_A
    GPIO_setPinConfig(GPIO_40_EQEP1_A);
    GPIO_setDirectionMode(40, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(40, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(40, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(40, 2);

    // GPIO41->QEP1_B
    GPIO_setPinConfig(GPIO_41_EQEP1_B);
    GPIO_setDirectionMode(41, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(41, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(41, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(41, 2);
*/
    // GPIO42->M1_DRV_MODE->6-PWM-Mode (GND)
    GPIO_setPinConfig(GPIO_42_GPIO42);
    GPIO_writePin(42, 0);
    GPIO_setDirectionMode(42, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(42, GPIO_PIN_TYPE_STD);

    // GPIO43->Reserve
    GPIO_setPinConfig(GPIO_43_GPIO43);
    GPIO_setDirectionMode(43, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(43, GPIO_PIN_TYPE_STD);

    // GPIO44->Reserve
    GPIO_setPinConfig(GPIO_44_GPIO44);
    GPIO_setDirectionMode(44, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(44, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(44, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(44, 2);

    // GPIO45->M2_DRV_MODE->6-PWM-Mode (GND)
    GPIO_setPinConfig(GPIO_45_GPIO45);
    GPIO_writePin(45, 0);
    GPIO_setDirectionMode(45, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(45, GPIO_PIN_TYPE_STD);

    // GPIO46->M2_DRV_CAL->Low
    GPIO_setPinConfig(GPIO_46_GPIO46);
    GPIO_writePin(46, 0);
    GPIO_setDirectionMode(46, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(46, GPIO_PIN_TYPE_STD);

    // GPIO47->Reserve
    GPIO_setPinConfig(GPIO_47_GPIO47);
    GPIO_setDirectionMode(47, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(47, GPIO_PIN_TYPE_STD);

    // GPIO48->M1_LED
    
   // GPIO_setPinConfig(GPIO_48_GPIO48);
    //PIO_writePin(48, 0);
    //GPIO_setDirectionMode(48, GPIO_DIR_MODE_OUT);
    //GPIO_setPadConfig(48, GPIO_PIN_TYPE_STD);

    // GPIO51->Reserve
    //GPIO_setPinConfig(GPIO_51_GPIO51);
    //GPIO_setDirectionMode(51, GPIO_DIR_MODE_IN);
    //GPIO_setPadConfig(51, GPIO_PIN_TYPE_STD);
     // GPIO34/GPIO51을 I2CB로 설정
//GPIO_SetupPinMux(34, GPIO_MUX_TO_GMUX, 3);  // GPIO34 -> I2CB_SDAB (MUX option 3)
//GPIO_SetupPinMux(51, GPIO_MUX_TO_GMUX, 3);  // GPIO51 -> I2CB_SCLB (MUX option 3)

GPIO_setPinConfig(GPIO_34_I2CB_SDA);
GPIO_setPinConfig(GPIO_51_I2CB_SCL);

  GPIO_setPadConfig(GPIO_34_I2CB_SDA, GPIO_PIN_TYPE_PULLUP|GPIO_PIN_TYPE_OD);
  GPIO_setPadConfig(GPIO_51_I2CB_SCL, GPIO_PIN_TYPE_PULLUP|GPIO_PIN_TYPE_OD);

// 입력 값 검증 (qualification) 설정 - 비동기 모드
GPIO_setQualificationMode(GPIO_34_I2CB_SDA, GPIO_QUAL_ASYNC);
GPIO_setQualificationMode(GPIO_51_I2CB_SCL, GPIO_QUAL_ASYNC);
    // GPIO55->HALL_V
    GPIO_setPinConfig(GPIO_55_GPIO55);
    GPIO_setDirectionMode(55, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(55, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(55, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(55, 2);

#if defined(SFRA_ENABLE)
    // GPIO56->SCIA_RX for SFRA
    GPIO_setPinConfig(GPIO_56_SCIB_TX);
    GPIO_setDirectionMode(56, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(56, GPIO_PIN_TYPE_STD);
#else  // !SFRA_ENABLE
    // GPIO56->M2_DRV_ENABLE
    GPIO_setPinConfig(GPIO_56_GPIO56);
    GPIO_writePin(56, 1);
    GPIO_setDirectionMode(56, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(56, GPIO_PIN_TYPE_STD);
#endif

    // GPIO57->HALL_W
    GPIO_setPinConfig(GPIO_57_GPIO57);
    GPIO_setDirectionMode(57, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(57, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(57, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(57, 2);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO58->SPIB_CLK->DAC128S_SCLK
    GPIO_setPinConfig(GPIO_58_SPIB_CLK);
    GPIO_setDirectionMode(58, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(58, GPIO_PIN_TYPE_STD);
#else
       GPIO_setPinConfig(GPIO_58_SPIB_CLK);
    GPIO_setDirectionMode(58, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(58, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIB

    // GPIO59->QEP1_INDEX
    GPIO_setPinConfig(GPIO_59_EQEP1_INDEX);
    GPIO_setDirectionMode(59, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(59, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(59, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(59, 2);

    // GPIO60->SPIB_SIMO->M2_DRV_SDI or DAC128S_SDI
    GPIO_setPinConfig(GPIO_60_SPIB_SIMO);
    GPIO_setDirectionMode(60, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(60, GPIO_PIN_TYPE_STD);

    // GPIO61->SPIB_SOMI->M2_DRV_SDO or DAC128S_SDO
    GPIO_setPinConfig(GPIO_61_SPIB_SOMI);
    GPIO_setDirectionMode(61, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(61, GPIO_PIN_TYPE_STD);
    // end of BSXL8316RT_REVA
//------------------------------------------------------------------------------
#elif defined(BSXL3PHGAN_REVA)
    // GPIO0->EPWM1A->M1_UH*
    GPIO_setPinConfig(GPIO_0_EPWM1_A);
    GPIO_setDirectionMode(0, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(0, GPIO_PIN_TYPE_STD);

    // GPIO1->EPWM1B->M1_UL*
    GPIO_setPinConfig(GPIO_1_EPWM1_B);
    GPIO_setDirectionMode(1, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(1, GPIO_PIN_TYPE_STD);

    // GPIO2->EPWM2A->M1_VH*
    GPIO_setPinConfig(GPIO_2_EPWM2_A);
    GPIO_setDirectionMode(2, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(2, GPIO_PIN_TYPE_STD);

    // GPIO3->EPWM2B->M1_VL*
    GPIO_setPinConfig(GPIO_3_EPWM2_B);
    GPIO_setDirectionMode(3, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(3, GPIO_PIN_TYPE_STD);

#if defined(CMD_CAN_EN)
    // GPIO4->CAN_TX
    GPIO_setPinConfig(GPIO_4_CANA_TX);
    GPIO_setDirectionMode(4, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_STD);
#else   // !CMD_CAN_EN
    // GPIO4->M2_nFAULT
    GPIO_setPinConfig(GPIO_4_GPIO4);
    GPIO_setDirectionMode(4, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_PULLUP);
#endif  // !CMD_CAN_EN

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIA)
    // GPIO5->DAC128S_SYNC->Use a Jumper connect to J2-12
    GPIO_setPinConfig(GPIO_5_SPIA_STE);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);
#elif defined(CMD_CAN_EN)
    // GPIO5->CAN_RX
    GPIO_setPinConfig(GPIO_5_CANA_RX);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);
#else   // !CMD_CAN_EN
    // GPIO5->Reserve GPIO
    GPIO_setPinConfig(GPIO_5_GPIO5);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);
#endif  // !(DAC128S_ENABLE && DAC128S_SPIA) & !CMD_CAN_EN

    // GPIO6->EPWM4A->M2_VH*
    GPIO_setPinConfig(GPIO_6_EPWM4_A);
    GPIO_setDirectionMode(6, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(6, GPIO_PIN_TYPE_STD);

    // GPIO7->EPWM4B->M2_VL*
    GPIO_setPinConfig(GPIO_7_EPWM4_B);
    GPIO_setDirectionMode(7, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(7, GPIO_PIN_TYPE_STD);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIA)
    // GPIO8->SPIA_SIMO->DAC12S_SDI
    GPIO_setPinConfig(GPIO_8_SPIA_SIMO);
    GPIO_setDirectionMode(8, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(8, GPIO_PIN_TYPE_STD);
#else
    // GPIO8->Reserve
    GPIO_setPinConfig(GPIO_8_GPIO8);
    GPIO_setDirectionMode(8, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(8, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIA

    // GPIO9->Reserve GPIO
    GPIO_setPinConfig(GPIO_9_GPIO9);
    GPIO_setDirectionMode(9, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(9, GPIO_PIN_TYPE_STD);

    // GPIO10->EPWM6A->M1_WH*
    GPIO_setPinConfig(GPIO_10_EPWM6_A);
    GPIO_setDirectionMode(10, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(10, GPIO_PIN_TYPE_STD);

    // GPIO11->EPWM6B->M1_WL*
    GPIO_setPinConfig(GPIO_11_EPWM6_B);
    GPIO_setDirectionMode(11, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(11, GPIO_PIN_TYPE_STD);

    // GPIO12->EPWM7A->M2_UH*
    GPIO_setPinConfig(GPIO_12_EPWM7_A);
    GPIO_setDirectionMode(12, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(12, GPIO_PIN_TYPE_STD);

    // GPIO13->EPWM7B->M2_UL*
    GPIO_setPinConfig(GPIO_13_EPWM7_B);
    GPIO_setDirectionMode(13, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(13, GPIO_PIN_TYPE_STD);

    // GPIO14->HALL_U
    GPIO_setPinConfig(GPIO_14_GPIO14);
    GPIO_setDirectionMode(14, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(14, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(14, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(14, 2);

#if defined(SFRA_ENABLE)
    // GPIO15->SCIA_RX for SFRA
    GPIO_setPinConfig(GPIO_15_SCIB_RX);
    GPIO_setDirectionMode(15, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(15, GPIO_PIN_TYPE_STD);
#else  // !SFRA_ENABLE
    // GPIO15->Reserve
    GPIO_setPinConfig(GPIO_15_GPIO15);
    GPIO_setDirectionMode(15, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(15, GPIO_PIN_TYPE_STD);
#endif

    // GPIO16->EPWM5A->M2_WH*
    GPIO_setPinConfig(GPIO_16_EPWM5_A);
    GPIO_setDirectionMode(16, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(16, GPIO_PIN_TYPE_STD);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIA)
    // GPIO17->SPIA_SOMI->DAC12S_SDO
    GPIO_setPinConfig(GPIO_17_SPIA_SOMI);
    GPIO_setDirectionMode(17, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(17, GPIO_PIN_TYPE_STD);
#else
    // GPIO17->Reserve
    GPIO_setPinConfig(GPIO_17_GPIO17);
    GPIO_setDirectionMode(17, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(17, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIA

    // GPIO18->Reserve
    GPIO_setPinConfig(GPIO_18_GPIO18);
    GPIO_setDirectionMode(18, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(18, GPIO_PIN_TYPE_STD);

    // GPIO19->Reserve
    GPIO_setPinConfig(GPIO_19_GPIO19);
    GPIO_setDirectionMode(19, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(19, GPIO_PIN_TYPE_STD);

    // GPIO20->LP_LED-RED
    GPIO_setAnalogMode(20, GPIO_ANALOG_DISABLED);
    GPIO_setPinConfig(GPIO_20_GPIO20);
    GPIO_writePin(20, 0);
    GPIO_setDirectionMode(20, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(20, GPIO_PIN_TYPE_STD);

    // GPIO21->M2_DRV_MODE->6-PWM-Mode (->GND)
    GPIO_setAnalogMode(21, GPIO_ANALOG_DISABLED);
    GPIO_setPinConfig(GPIO_21_GPIO21);
    GPIO_writePin(21, 0);
    GPIO_setDirectionMode(21, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(21, GPIO_PIN_TYPE_STD);

    // GPIO22->nEN_uC (only for TEST)
    GPIO_setPinConfig(GPIO_22_GPIO22);
    GPIO_writePin(20, 0);
    GPIO_setDirectionMode(22, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(22, GPIO_PIN_TYPE_STD);

    // GPIO23->M1_DRV_CAL (Low)
    GPIO_setPinConfig(GPIO_23_GPIO23);
    GPIO_writePin(23, 0);
    GPIO_setDirectionMode(23, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(23, GPIO_PIN_TYPE_STD);

    // GPIO24->M1_DRV_nFAULT*
    GPIO_setPinConfig(GPIO_24_GPIO24);
    GPIO_setDirectionMode(24, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(24, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(24, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(24, 2);

    // GPIO25->Reserve
    GPIO_setPinConfig(GPIO_25_GPIO25);
    GPIO_setDirectionMode(25, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(25, GPIO_PIN_TYPE_STD);

    // GPIO26->Reserve
    GPIO_setPinConfig(GPIO_26_GPIO26);
    GPIO_setDirectionMode(26, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(26, GPIO_PIN_TYPE_STD);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO27->SPIB_STE->DAC128S_SYNC
    GPIO_setPinConfig(GPIO_27_SPIB_STE);
    GPIO_setDirectionMode(27, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(27, GPIO_PIN_TYPE_STD);
#else
    // GPIO27->Reserve
    GPIO_setPinConfig(GPIO_27_GPIO27);
    GPIO_setDirectionMode(27, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(27, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIB

    // GPIO28->HAL_GPIO_ISR_M1
    GPIO_setPinConfig(GPIO_28_GPIO28);
    GPIO_writePin(28, 1);
    GPIO_setDirectionMode(28, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(28, GPIO_PIN_TYPE_PULLUP);

      /*
     // GPIO29->M1_DRV_ENABLE*
     GPIO_setPinConfig(GPIO_29_GPIO29);
    GPIO_writePin(29, 1);
    GPIO_setDirectionMode(29, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(29, GPIO_PIN_TYPE_PULLUP);
     */
    // GPIO30->Reserve
    GPIO_setPinConfig(GPIO_30_GPIO30);
    GPIO_setDirectionMode(30, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(30, GPIO_PIN_TYPE_STD);

    // GPIO31->M2_LED
    GPIO_setPinConfig(GPIO_31_GPIO31);
    GPIO_writePin(31, 0);
    GPIO_setDirectionMode(31, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(31, GPIO_PIN_TYPE_STD);

    // GPIO32->Reserve
    GPIO_setPinConfig(GPIO_32_GPIO32);
    GPIO_setDirectionMode(32, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(32, GPIO_PIN_TYPE_STD);

    // GPIO33->M1_OT
    GPIO_setPinConfig(GPIO_33_GPIO33);
    GPIO_setDirectionMode(33, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(33, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(33, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(33, 2);

    // GPIO34->Reserve
    GPIO_setPinConfig(GPIO_34_GPIO34);
    GPIO_setDirectionMode(34, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(34, GPIO_PIN_TYPE_STD);

#if !defined(MOTOR2_ISBLDC)
    // GPIO35->EPWM5B->M2_WL
    GPIO_setPinConfig(GPIO_35_EPWM5_B);
    GPIO_setDirectionMode(35, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(35, GPIO_PIN_TYPE_STD);
#else
    GPIO_setPinConfig(GPIO_35_GPIO35);
    GPIO_setDirectionMode(35, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(35, GPIO_PIN_TYPE_STD);
#endif // !MOTOR2_ISBLDC

    // GPIO37->uC_EN
    GPIO_setPinConfig(GPIO_37_GPIO37);
    GPIO_writePin(37, 0);
    GPIO_setDirectionMode(37, GPIO_DIR_MODE_OUT);
//    GPIO_writePin(37, 0);
//    GPIO_setDirectionMode(37, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(37, GPIO_PIN_TYPE_STD);

    // GPIO39->HAL_GPIO_ISR_M1
    GPIO_setPinConfig(GPIO_39_GPIO39);
    GPIO_setDirectionMode(39, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(39, GPIO_PIN_TYPE_STD);

    // GPIO40->QEP1_A
    GPIO_setPinConfig(GPIO_40_EQEP1_A);
    GPIO_setDirectionMode(40, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(40, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(40, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(40, 2);

    // GPIO41->QEP1_B
    GPIO_setPinConfig(GPIO_41_EQEP1_B);
    GPIO_setDirectionMode(41, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(41, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(41, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(41, 2);

    // GPIO42->M2_DRV_MODE->6-PWM-Mode (GND)
    GPIO_setPinConfig(GPIO_42_GPIO42);
    GPIO_writePin(42, 0);
    GPIO_setDirectionMode(42, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(42, GPIO_PIN_TYPE_STD);

    // GPIO43->Reserve
    GPIO_setPinConfig(GPIO_43_GPIO43);
    GPIO_setDirectionMode(43, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(43, GPIO_PIN_TYPE_STD);

    // GPIO44->Reserve
    GPIO_setPinConfig(GPIO_44_GPIO44);
    GPIO_setDirectionMode(44, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(44, GPIO_PIN_TYPE_STD);

    // GPIO45->M2_DRV_MODE->6-PWM-Mode (GND)
    GPIO_setPinConfig(GPIO_45_GPIO45);
    GPIO_writePin(45, 0);
    GPIO_setDirectionMode(45, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(45, GPIO_PIN_TYPE_STD);

    // GPIO46->M2_DRV_CAL->Low
    GPIO_setPinConfig(GPIO_46_GPIO46);
    GPIO_writePin(46, 0);
    GPIO_setDirectionMode(46, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(46, GPIO_PIN_TYPE_STD);

    // GPIO47->Reserve
    GPIO_setPinConfig(GPIO_47_GPIO47);
    GPIO_setDirectionMode(47, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(47, GPIO_PIN_TYPE_STD);

    // GPIO48->M1_LED
    GPIO_setPinConfig(GPIO_48_GPIO48);
    GPIO_writePin(48, 0);
    GPIO_setDirectionMode(48, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(48, GPIO_PIN_TYPE_STD);

    // GPIO51->Reserve
    GPIO_setPinConfig(GPIO_51_GPIO51);
    GPIO_setDirectionMode(51, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(51, GPIO_PIN_TYPE_STD);

    // GPIO55->HALL_V
    GPIO_setPinConfig(GPIO_55_GPIO55);
    GPIO_setDirectionMode(55, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(55, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(55, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(55, 2);

#if defined(SFRA_ENABLE)
    // GPIO56->SCIA_RX for SFRA
    GPIO_setPinConfig(GPIO_56_SCIB_TX);
    GPIO_setDirectionMode(56, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(56, GPIO_PIN_TYPE_STD);
#else  // !SFRA_ENABLE
    // GPIO56->M2_DRV_ENABLE
    GPIO_setPinConfig(GPIO_56_GPIO56);
    GPIO_writePin(56, 1);
    GPIO_setDirectionMode(56, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(56, GPIO_PIN_TYPE_STD);
#endif

    // GPIO57->HALL_W
    GPIO_setPinConfig(GPIO_57_GPIO57);
    GPIO_setDirectionMode(57, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(57, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(57, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(57, 2);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO58->SPIB_CLK->DAC128S_SCLK
    GPIO_setPinConfig(GPIO_58_SPIB_CLK);
    GPIO_setDirectionMode(58, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(58, GPIO_PIN_TYPE_STD);
#else
    // GPIO58->Reserve GPIO
    GPIO_setPinConfig(GPIO_58_GPIO58);
    GPIO_setDirectionMode(58, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(58, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIB

    // GPIO59->QEP1_INDEX
    GPIO_setPinConfig(GPIO_59_EQEP1_INDEX);
    GPIO_setDirectionMode(59, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(59, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(59, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(59, 2);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO60->SPIB_SIMO->DAC128S_SDI
    GPIO_setPinConfig(GPIO_60_SPIB_SIMO);
    GPIO_setDirectionMode(60, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(60, GPIO_PIN_TYPE_STD);
#else   // DAC128S_ENABLE && DAC128S_SPIB
    // GPIO60->Reserve
    GPIO_setPinConfig(GPIO_60_GPIO60);
    GPIO_setDirectionMode(60, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(60, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIB

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO61->SPIB_SOMI->DAC128S_SDO
    GPIO_setPinConfig(GPIO_61_SPIB_SOMI);
    GPIO_setDirectionMode(61, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(61, GPIO_PIN_TYPE_STD);
#else   // DAC128S_ENABLE && DAC128S_SPIB
    // GPIO61->Reserve
    GPIO_setPinConfig(GPIO_61_GPIO61);
    GPIO_setDirectionMode(61, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(61, GPIO_PIN_TYPE_STD);
#endif  // DAC128S_ENABLE && DAC128S_SPIB
    // end of BSXL3PHGAN_REVA

//------------------------------------------------------------------------------
#elif defined(HVMTRPFC_REV1P1)
    // GPIO0->EPWM1A->M1_UH
    GPIO_setPinConfig(GPIO_0_EPWM1_A);
    GPIO_setDirectionMode(0, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(0, GPIO_PIN_TYPE_STD);

    // GPIO1->EPWM1B->M1_UL
    GPIO_setPinConfig(GPIO_1_EPWM1_B);
    GPIO_setDirectionMode(1, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(1, GPIO_PIN_TYPE_STD);

    // GPIO2->EPWM2A->M1_VH
    GPIO_setPinConfig(GPIO_2_EPWM2_A);
    GPIO_setDirectionMode(2, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(2, GPIO_PIN_TYPE_STD);

    // GPIO3->EPWM2B->M1_VL
    GPIO_setPinConfig(GPIO_3_EPWM2_B);
    GPIO_setDirectionMode(3, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(3, GPIO_PIN_TYPE_STD);

    // GPIO4->EPWM3A->M1_WH
    GPIO_setPinConfig(GPIO_4_EPWM3_A);
    GPIO_setDirectionMode(4, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_STD);

    // GPIO5->EPWM3B->M1_WL
    GPIO_setPinConfig(GPIO_5_EPWM3_B);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);

    // GPIO6->EPWM4A->PFC_1H
    GPIO_setPinConfig(GPIO_6_EPWM4_A);
    GPIO_setDirectionMode(6, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(6, GPIO_PIN_TYPE_STD);

    // GPIO7->EPWM4B->PFC_1L
    GPIO_setPinConfig(GPIO_7_EPWM4_B);
    GPIO_setDirectionMode(7, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(7, GPIO_PIN_TYPE_STD);

    // GPIO8->Reserve
    GPIO_setPinConfig(GPIO_8_GPIO8);
    GPIO_setDirectionMode(8, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(8, GPIO_PIN_TYPE_STD);

    // GPIO09->CLR-Fault
    GPIO_setPinConfig(GPIO_9_GPIO9);
    GPIO_writePin(9, 1);
    GPIO_setDirectionMode(9, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(9, GPIO_PIN_TYPE_PULLUP);

    // PWM6A->DAC1
    GPIO_setPinConfig(GPIO_10_EPWM6_A);
    GPIO_setDirectionMode(10, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(10, GPIO_PIN_TYPE_STD);

    // PWM6B->DAC2
    GPIO_setPinConfig(GPIO_11_EPWM6_B);
    GPIO_setDirectionMode(11, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(11, GPIO_PIN_TYPE_STD);

    // PWM7A->DAC3
    GPIO_setPinConfig(GPIO_12_EPWM7_A);
    GPIO_setDirectionMode(12, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(12, GPIO_PIN_TYPE_STD);

    // PWM7B->DAC4
    GPIO_setPinConfig(GPIO_13_EPWM7_B);
    GPIO_setDirectionMode(13, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(13, GPIO_PIN_TYPE_STD);

    // GPIO14->LED2 on HVKIT
    GPIO_setPinConfig(GPIO_14_GPIO14);
    GPIO_writePin(14, 1);
    GPIO_setDirectionMode(14, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(14, GPIO_PIN_TYPE_STD);

    // GPIO15->Reserve
    GPIO_setPinConfig(GPIO_15_GPIO15);
    GPIO_setDirectionMode(15, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(15, GPIO_PIN_TYPE_STD);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIA)
    // GPIO16->SPIA_SIMO for DAC80504
    GPIO_setPinConfig(GPIO_16_SPIA_SIMO);
    GPIO_setDirectionMode(16, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(16, GPIO_PIN_TYPE_STD);
#else   //!DAC_ENABLE
    // GPIO16->Reserve
    GPIO_setPinConfig(GPIO_16_GPIO16);
    GPIO_setDirectionMode(16, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(16, GPIO_PIN_TYPE_STD);
#endif  //!DAC_ENABLE

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIA)
    // GPIO17->SPIA_SOMI for DAC80504
    GPIO_setPinConfig(GPIO_17_SPIA_SOMI);
    GPIO_setDirectionMode(17, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(17, GPIO_PIN_TYPE_STD);
#else   //!DAC_ENABLE
    // GPIO17->Reserve
    GPIO_setPinConfig(GPIO_17_GPIO17);
    GPIO_setDirectionMode(17, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(17, GPIO_PIN_TYPE_STD);
#endif  //!DAC_ENABLE

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIA)
    // GPIO18->SPIA_CLK for DAC80504
    GPIO_setPinConfig(GPIO_18_SPIA_CLK);
    GPIO_setDirectionMode(18, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(18, GPIO_PIN_TYPE_STD);
#else   //!DAC_ENABLE
    // GPIO18->Reserve
    GPIO_setPinConfig(GPIO_18_GPIO18);
    GPIO_setDirectionMode(18, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(18, GPIO_PIN_TYPE_STD);
#endif  //!DAC_ENABLE

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIA)
    // GPIO19->SPIA_STE for DAC80504
    GPIO_setPinConfig(GPIO_19_SPIA_STE);
    GPIO_setDirectionMode(19, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(19, GPIO_PIN_TYPE_STD);
#else   //!DAC_ENABLE
    // GPIO19->Reserve
    GPIO_setPinConfig(GPIO_19_GPIO19);
    GPIO_setDirectionMode(19, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(19, GPIO_PIN_TYPE_STD);
#endif  //!DAC_ENABLE

#if defined(MOTOR1_ENC)
    // GPIO20->EQEP1_A
    GPIO_setPinConfig(GPIO_20_EQEP1_A);
    GPIO_setDirectionMode(20, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(20, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(20, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(20, 2);
#elif defined(MOTOR1_HALL)
    // GPIO20->ECAP1->HALL_U
    GPIO_setPinConfig(GPIO_20_GPIO20);
    GPIO_setDirectionMode(20, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(20, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(20, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(20, 2);
#else   // !MOTOR1_ENC & !MOTOR1_HALL
    // GPIO20->Reserve
    GPIO_setPinConfig(GPIO_20_GPIO20);
    GPIO_setDirectionMode(20, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(20, GPIO_PIN_TYPE_STD);
#endif   // !MOTOR1_ENC & !MOTOR1_HALL

#if defined(MOTOR1_ENC)
    // GPIO21->EQEP1_A
    GPIO_setPinConfig(GPIO_21_EQEP1_B);
    GPIO_setDirectionMode(21, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(21, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(21, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(21, 2);
#elif defined(MOTOR1_HALL)
    // GPIO21->ECAP2->HALL_V
    GPIO_setPinConfig(GPIO_21_GPIO21);
    GPIO_setDirectionMode(21, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(21, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(21, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(21, 2);
#else   // !MOTOR1_ENC & !MOTOR1_HALL
    // GPIO21->Reserve
    GPIO_setPinConfig(GPIO_21_GPIO21);
    GPIO_setDirectionMode(21, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(21, GPIO_PIN_TYPE_STD);
#endif   // !MOTOR1_ENC & !MOTOR1_HALL

    // GPIO22->Reserve/CAP_CMD
    GPIO_setPinConfig(GPIO_22_GPIO22);
    GPIO_setDirectionMode(22, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(22, GPIO_PIN_TYPE_STD);

#if defined(MOTOR1_ENC)
    // GPIO23->EQEP1_I
    GPIO_setPinConfig(GPIO_23_EQEP1_INDEX);
    GPIO_setDirectionMode(23, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(23, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(23, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(23, 2);
#elif defined(MOTOR1_HALL)
    // GPIO23->ECAP3->HALL_W
    GPIO_setPinConfig(GPIO_23_GPIO23);
    GPIO_setDirectionMode(23, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(23, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(23, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(23, 2);
#else   // !MOTOR1_ENC & !MOTOR1_HALL
    // GPIO23->Reserve
    GPIO_setPinConfig(GPIO_23_GPIO23);
    GPIO_setDirectionMode(23, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(23, GPIO_PIN_TYPE_STD);
#endif   // !MOTOR1_ENC & !MOTOR1_HALL

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO24->SPIB_SIMO for DAC80504
    GPIO_setPinConfig(GPIO_24_SPIB_SIMO);
    GPIO_setDirectionMode(24, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(24, GPIO_PIN_TYPE_STD);
#else   //!DAC_ENABLE
    // GPIO24->CAP1->HALL_U
    GPIO_setPinConfig(GPIO_24_GPIO24);
    GPIO_setDirectionMode(24, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(24, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(24, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(24, 2);
#endif   //!DAC_ENABLE

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO24->SPIB_SOMI for DAC80504
    GPIO_setPinConfig(GPIO_25_SPIB_SOMI);
    GPIO_setDirectionMode(25, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(25, GPIO_PIN_TYPE_STD);
#else   //!DAC_ENABLE
    // GPIO25->CAP2->HALL_V
    GPIO_setPinConfig(GPIO_25_GPIO25);
    GPIO_setDirectionMode(25, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(25, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(25, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(25, 1);
#endif   //!DAC_ENABLE

    // GPIO26->CAP3->HALL_U
    GPIO_setPinConfig(GPIO_26_GPIO26);
    GPIO_setDirectionMode(26, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(26, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(26, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(26, 1);

    // GPIO27->PFC ISR Executing Time
    GPIO_setPinConfig(GPIO_27_GPIO27);
    GPIO_setDirectionMode(27, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(27, GPIO_PIN_TYPE_STD);

    // GPIO28->SCIA_RX
    GPIO_setPinConfig(GPIO_28_SCIA_RX);
    GPIO_setDirectionMode(28, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(28, GPIO_PIN_TYPE_STD);

    // GPIO28->SCIA_TX
    GPIO_setPinConfig(GPIO_29_SCIA_TX);
    GPIO_setDirectionMode(29, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(29, GPIO_PIN_TYPE_STD);

    // GPIO30->CANRX
    GPIO_setPinConfig(GPIO_30_CANA_RX);
    GPIO_setDirectionMode(30, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(30, GPIO_PIN_TYPE_STD);

    // GPIO31->CAN-TX
    GPIO_setPinConfig(GPIO_31_CANA_TX);
    GPIO_setDirectionMode(31, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(31, GPIO_PIN_TYPE_STD);

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO32->SPIB_CLK for DAC128S085
    GPIO_setPinConfig(GPIO_32_SPIB_CLK);
    GPIO_setDirectionMode(32, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(32, GPIO_PIN_TYPE_STD);
#else  // !DAC_ENABLE
    // GPIO32->Reserve
    GPIO_setPinConfig(GPIO_32_GPIO32);
    GPIO_setDirectionMode(32, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(32, GPIO_PIN_TYPE_STD);
#endif  // !DAC_ENABLE

#if defined(DAC128S_ENABLE) && defined(DAC128S_SPIB)
    // GPIO32->SPIB_STE for DAC128S085
    GPIO_setPinConfig(GPIO_33_SPIB_STE);
    GPIO_setDirectionMode(33, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(33, GPIO_PIN_TYPE_STD);
#else  // !DAC_ENABLE
    // GPIO33->Reserve
    GPIO_setPinConfig(GPIO_33_GPIO33);
    GPIO_setDirectionMode(33, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(33, GPIO_PIN_TYPE_STD);
#endif  // !DAC_ENABLE

    // GPIO34->LED-D2 on Control Card
    GPIO_setPinConfig(GPIO_34_GPIO34);
    GPIO_writePin(34, 1);
    GPIO_setDirectionMode(34, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(34, GPIO_PIN_TYPE_STD);

    // TDI
    GPIO_setPinConfig(GPIO_35_TDI);

    // TDO
    GPIO_setPinConfig(GPIO_37_TDO);

    // GPIO39->nFault
    GPIO_setPinConfig(GPIO_39_GPIO39);
    GPIO_setDirectionMode(39, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(39, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(39, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(39, 1);

    // GPIO40->nFAULT*
    
    GPIO_setPinConfig(GPIO_40_GPIO40);
    GPIO_setDirectionMode(40, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(40, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(40, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(40, 2);

    // Reserve
    GPIO_setPinConfig(GPIO_41_GPIO41);
    GPIO_setDirectionMode(41, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(41, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(41, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(41, 2);

    // GPIO42->LED1 on HVKIT
    GPIO_setPinConfig(GPIO_42_GPIO42);
    GPIO_writePin(42, 1);
    GPIO_setDirectionMode(42, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(42, GPIO_PIN_TYPE_STD);

    // GPIO43->Reserve
    GPIO_setPinConfig(GPIO_43_GPIO43);
    GPIO_setDirectionMode(43, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(43, GPIO_PIN_TYPE_STD);

    // GPIO44->Reserve
    GPIO_setPinConfig(GPIO_44_GPIO44);
    GPIO_setDirectionMode(44, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(44, GPIO_PIN_TYPE_STD);

    // GPIO45->Reserve
    GPIO_setPinConfig(GPIO_45_GPIO45);
    GPIO_setDirectionMode(45, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(45, GPIO_PIN_TYPE_STD);

    // GPIO46->Reserve
    GPIO_setPinConfig(GPIO_46_GPIO46);
    GPIO_setDirectionMode(46, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(46, GPIO_PIN_TYPE_STD);

    // GPIO48->LED1 on HVKIT*
    GPIO_setPinConfig(GPIO_48_GPIO48);
    GPIO_writePin(48, 1);
    GPIO_setDirectionMode(48, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(48, GPIO_PIN_TYPE_STD);

    // GPIO59->OCP of IPM
    
    GPIO_setPinConfig(GPIO_59_GPIO59);
    GPIO_setDirectionMode(59, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(59, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(59, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(59, 2);

    // end of HVMTRPFC_REV1P1
//------------------------------------------------------------------------------
#elif defined(BSXL8305RS_REVA)
    // GPIO0->EPWM1A->M1_UH*
    GPIO_setPinConfig(GPIO_0_EPWM1_A);
    GPIO_setDirectionMode(0, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(0, GPIO_PIN_TYPE_STD);

    // GPIO1->EPWM1B->M1_UL*
    GPIO_setPinConfig(GPIO_1_EPWM1_B);
    GPIO_setDirectionMode(1, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(1, GPIO_PIN_TYPE_STD);

    // GPIO2->EPWM2A->M1_VH*
    GPIO_setPinConfig(GPIO_2_EPWM2_A);
    GPIO_setDirectionMode(2, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(2, GPIO_PIN_TYPE_STD);

    // GPIO3->EPWM2B->M1_VL*
    GPIO_setPinConfig(GPIO_3_EPWM2_B);
    GPIO_setDirectionMode(3, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(3, GPIO_PIN_TYPE_STD);

    // GPIO4->EPWM3A->M1_WH*
    GPIO_setPinConfig(GPIO_4_EPWM3_A);
    GPIO_setDirectionMode(4, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_STD);

    // GPIO5->M1_DRV_CS
    GPIO_setPinConfig(GPIO_5_SPIA_STE);
//    GPIO_setPinConfig(GPIO_5_GPIO5);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_PULLUP);

    // GPIO6->EPWM4A->M2_VH*
    GPIO_setPinConfig(GPIO_6_EPWM4_A);
    GPIO_setDirectionMode(6, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(6, GPIO_PIN_TYPE_STD);

    // GPIO7->EPWM4B->M2_VL*
    GPIO_setPinConfig(GPIO_7_EPWM4_B);
    GPIO_setDirectionMode(7, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(7, GPIO_PIN_TYPE_STD);

    // GPIO8->M1_DRV_WAKE
    GPIO_setPinConfig(GPIO_8_GPIO8);
    GPIO_writePin(8, 1);
    GPIO_setDirectionMode(8, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(8, GPIO_PIN_TYPE_PULLUP);

    // GPIO09->M1_DRV_SCLK*
    GPIO_setPinConfig(GPIO_9_SPIA_CLK);
    GPIO_setDirectionMode(9, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(9, GPIO_PIN_TYPE_PULLUP);

    // GPIO10->SPIA_SOMI->M1_DRV_SDO*
    GPIO_setPinConfig(GPIO_10_SPIA_SOMI);
    GPIO_setDirectionMode(10, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(10, GPIO_PIN_TYPE_PULLUP);

    // GPIO11->SPIA_SIMO->M1_DRV_SDI*
    GPIO_setPinConfig(GPIO_11_SPIA_SIMO);
    GPIO_setDirectionMode(11, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(11, GPIO_PIN_TYPE_PULLUP);

    // GPIO12->EPWM7A->M2_UH*
    GPIO_setPinConfig(GPIO_12_EPWM7A);
    GPIO_setDirectionMode(12, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(12, GPIO_PIN_TYPE_STD);

    // GPIO13->EPWM7B->M2_UL*
    GPIO_setPinConfig(GPIO_13_EPWM7B);
    GPIO_setDirectionMode(13, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(13, GPIO_PIN_TYPE_STD);

    // GPIO14->EQEP2_A
    GPIO_setPinConfig(GPIO_14_EQEP2_A);
    GPIO_setDirectionMode(14, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(14, GPIO_PIN_TYPE_STD);

    // GPIO15->EPWM3B->M1_WL*
    GPIO_setPinConfig(GPIO_15_EPWM3_B);
    GPIO_setDirectionMode(15, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(15, GPIO_PIN_TYPE_STD);

    // GPIO16->EPWM5A->M2_WH*
    GPIO_setPinConfig(GPIO_16_EPWM5_A);
    GPIO_setDirectionMode(16, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(16, GPIO_PIN_TYPE_STD);

    // GPIO17->EPWM5B->M2_WL*
    GPIO_setPinConfig(GPIO_17_EPWM5_B);
    GPIO_setDirectionMode(17, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(17, GPIO_PIN_TYPE_STD);

    // GPIO18->Reserve
    GPIO_setPinConfig(GPIO_18_GPIO18);
    GPIO_setDirectionMode(18, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(18, GPIO_PIN_TYPE_STD);

    // GPIO19->M1_DRV_LED
    GPIO_setPinConfig(GPIO_19_GPIO19);
    GPIO_writePin(19, 1);
    GPIO_setDirectionMode(19, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(19, GPIO_PIN_TYPE_STD);

    // GPIO22->SPIB_CLK->M2_DRV_SCLK
    GPIO_setPinConfig(GPIO_22_SPIB_CLK);
    GPIO_setDirectionMode(22, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(22, GPIO_PIN_TYPE_PULLUP);

    // GPIO23->M1_DRV_ENABLE*
    GPIO_setPinConfig(GPIO_23_GPIO23);
    GPIO_writePin(23, 1);
    GPIO_setDirectionMode(23, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(23, GPIO_PIN_TYPE_PULLUP);

    // GPIO24->M2_DRV_ENABLE*
    GPIO_setPinConfig(GPIO_24_GPIO24);
    GPIO_writePin(24, 1);
    GPIO_setDirectionMode(24, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(24, GPIO_PIN_TYPE_STD);

    // GPIO25->EQEP2_B
    GPIO_setPinConfig(GPIO_25_EQEP2_B);
    GPIO_setDirectionMode(25, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(25, GPIO_PIN_TYPE_STD);

    // GPIO26->EQEP2_INDEX
    GPIO_setPinConfig(GPIO_26_EQEP2_INDEX);
    GPIO_setDirectionMode(26, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(26, GPIO_PIN_TYPE_STD);

    // GPIO27->PFC ISR Executing Time
    GPIO_setPinConfig(GPIO_27_GPIO27);
    GPIO_setDirectionMode(27, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(27, GPIO_PIN_TYPE_STD);

    // GPIO28->M1_DRV_nFault
    GPIO_setPinConfig(GPIO_28_GPIO28);
    GPIO_writePin(28, 1);
    GPIO_setDirectionMode(28, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(28, GPIO_PIN_TYPE_STD);

    // GPIO29->M1_DRV_nFault
    GPIO_setPinConfig(GPIO_29_GPIO29);
    GPIO_setDirectionMode(29, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(29, GPIO_PIN_TYPE_STD);

    // GPIO30->SPIB_SIMO->M2_DRV_SDI*
    GPIO_setPinConfig(GPIO_30_SPIB_SIMO);
    GPIO_setDirectionMode(30, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(30, GPIO_PIN_TYPE_PULLUP);

    // GPIO31->SPIB_SOMI->M2_DRV_SDO*
    GPIO_setPinConfig(GPIO_31_SPIB_SOMI);
    GPIO_setDirectionMode(31, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(31, GPIO_PIN_TYPE_PULLUP);

    // GPIO32->M2_DRV_WAKE*
    GPIO_setPinConfig(GPIO_32_GPIO32);
    GPIO_writePin(32, 1);
    GPIO_setDirectionMode(32, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(32, GPIO_PIN_TYPE_PULLUP);

    // GPIO33->M2_DRV_SCS
    GPIO_setPinConfig(GPIO_33_SPIB_STE);
//    GPIO_setPinConfig(GPIO_33_GPIO33);
    GPIO_setDirectionMode(33, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(33, GPIO_PIN_TYPE_PULLUP);

    // GPIO34->LED on LaunchPad
    GPIO_setPinConfig(GPIO_34_GPIO34);
    GPIO_writePin(34, 1);
    GPIO_setDirectionMode(34, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(34, GPIO_PIN_TYPE_PULLUP);

    // GPIO35->M2_DRV_nFAULT*
    GPIO_setPinConfig(GPIO_35_GPIO35);
    GPIO_setDirectionMode(35, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(35, GPIO_PIN_TYPE_PULLUP);

    // GPIO37->EQEP1_B
    GPIO_setPinConfig(GPIO_37_EQEP1_B);
    GPIO_setDirectionMode(37, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(37, GPIO_PIN_TYPE_STD);

    // GPIO39->M2_DRV_ENABLE*
    GPIO_setPinConfig(GPIO_39_GPIO39);
    GPIO_writePin(39, 1);
    GPIO_setDirectionMode(39, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(39, GPIO_PIN_TYPE_PULLUP);

    // GPIO40->Reserve
    GPIO_setPinConfig(GPIO_40_GPIO40);
    GPIO_setDirectionMode(40, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(40, GPIO_PIN_TYPE_STD);

    // GPIO41->M1_DRV_nFAULT
    GPIO_setPinConfig(GPIO_41_EQEP1B);
    GPIO_setDirectionMode(41, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(41, GPIO_PIN_TYPE_STD);

    // GPIO42->M1_DRV_MODE#
    GPIO_setPinConfig(GPIO_42_GPIO42);
    GPIO_setDirectionMode(42, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(42, GPIO_PIN_TYPE_STD);

    // GPIO43->EQEP1_INDEX!
    GPIO_setPinConfig(GPIO_43_EQEP1_INDEX);
    GPIO_setDirectionMode(43, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(43, GPIO_PIN_TYPE_STD);

    // GPIO44->Reserve
    GPIO_setPinConfig(GPIO_41_GPIO41);
    GPIO_setDirectionMode(44, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(44, GPIO_PIN_TYPE_STD);

    // GPIO45->M2_DRV_MODE#, N/A
    GPIO_setPinConfig(GPIO_45_GPIO45);
    GPIO_setDirectionMode(45, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(45, GPIO_PIN_TYPE_STD);

    // GPIO46->Reserve
    GPIO_setPinConfig(GPIO_46_GPIO46);
    GPIO_setDirectionMode(46, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(46, GPIO_PIN_TYPE_STD);
    // end of BSXL8305RS_REVA
//------------------------------------------------------------------------------
#elif defined(DRV8301EVM_REVD)
    // GPIO0->EPWM1A->M1_UH
    GPIO_setPinConfig(GPIO_0_EPWM1_A);
    GPIO_setDirectionMode(0, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(0, GPIO_PIN_TYPE_STD);

    // GPIO1->EPWM1B->M1_UL
    GPIO_setPinConfig(GPIO_1_EPWM1_B);
    GPIO_setDirectionMode(1, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(1, GPIO_PIN_TYPE_STD);

    // GPIO2->EPWM2A->M1_VH
    GPIO_setPinConfig(GPIO_2_EPWM2_A);
    GPIO_setDirectionMode(2, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(2, GPIO_PIN_TYPE_STD);

    // GPIO3->EPWM2B->M1_VL
    GPIO_setPinConfig(GPIO_3_EPWM2_B);
    GPIO_setDirectionMode(3, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(3, GPIO_PIN_TYPE_STD);

    // GPIO4->EPWM3A->M1_WH
    GPIO_setPinConfig(GPIO_4_EPWM3_A);
    GPIO_setDirectionMode(4, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_STD);

    // GPIO5->EPWM3B->M1_WL
    GPIO_setPinConfig(GPIO_5_EPWM3_B);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);

    // GPIO6->DAC-PWM4
    GPIO_setPinConfig(GPIO_6_EPWM4_A);
    GPIO_setDirectionMode(6, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(6, GPIO_PIN_TYPE_STD);

    // GPIO7->STOP
    GPIO_setPinConfig(GPIO_7_GPIO7);
    GPIO_setDirectionMode(7, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(7, GPIO_PIN_TYPE_STD);

    // GPIO8->DAC-PWM3
    GPIO_setPinConfig(GPIO_8_EPWM5_A);
    GPIO_setDirectionMode(8, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(8, GPIO_PIN_TYPE_STD);

    // GPIO09->START
    GPIO_setPinConfig(GPIO_9_GPIO9);
    GPIO_setDirectionMode(9, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(9, GPIO_PIN_TYPE_STD);

    // GPIO10->DAC-PWM1
    GPIO_setPinConfig(GPIO_10_EPWM6A);
    GPIO_setDirectionMode(10, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(10, GPIO_PIN_TYPE_STD);

    // GPIO11->DAC-PWM2
    GPIO_setPinConfig(GPIO_11_EPWM6B);
    GPIO_setDirectionMode(11, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(11, GPIO_PIN_TYPE_STD);

    // GPIO12->J5-12->OCTW
    GPIO_setPinConfig(GPIO_12_GPIO12);
    GPIO_setDirectionMode(12, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(12, GPIO_PIN_TYPE_STD);

    // GPIO13->Reserve
    GPIO_setPinConfig(GPIO_13_GPIO13);
    GPIO_setDirectionMode(13, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(13, GPIO_PIN_TYPE_STD);

    // GPIO14->J5-9->FAULTn
    GPIO_setPinConfig(GPIO_14_GPIO14);
    GPIO_setDirectionMode(14, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(14, GPIO_PIN_TYPE_STD);

    // GPIO15->Reserve
    GPIO_setPinConfig(GPIO_15_GPIO15);
    GPIO_setDirectionMode(15, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(15, GPIO_PIN_TYPE_STD);

    // GPIO16->Reserve
    GPIO_setPinConfig(GPIO_16_GPIO16);
    GPIO_setDirectionMode(16, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(16, GPIO_PIN_TYPE_STD);

    // GPIO17->Reserve
    GPIO_setPinConfig(GPIO_17_GPIO17);
    GPIO_setDirectionMode(17, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(17, GPIO_PIN_TYPE_STD);

    // GPIO18->Reserve
    GPIO_setPinConfig(GPIO_18_GPIO18);
    GPIO_setDirectionMode(18, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(18, GPIO_PIN_TYPE_STD);

    // GPIO19->Reserve
    GPIO_setPinConfig(GPIO_19_GPIO19);
    GPIO_setDirectionMode(19, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(19, GPIO_PIN_TYPE_STD);

    // GPIO22->STATUS
    GPIO_setPinConfig(GPIO_22_GPIO22);
    GPIO_writePin(22, 1);
    GPIO_setDirectionMode(22, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(22, GPIO_PIN_TYPE_STD);

    // GPIO23->J5-6->EN_GATE
    GPIO_setPinConfig(GPIO_23_GPIO23);
    GPIO_writePin(23, 1);
    GPIO_setDirectionMode(23, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(23, GPIO_PIN_TYPE_PULLUP);

    // GPIO24->SDI
    GPIO_setPinConfig(GPIO_24_SPIB_SIMO);
    GPIO_setDirectionMode(24, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(24, GPIO_PIN_TYPE_PULLUP);

    // GPIO25->SDO
    GPIO_setPinConfig(GPIO_25_SPIB_SOMI);
    GPIO_setDirectionMode(25, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(25, GPIO_PIN_TYPE_PULLUP);

    // GPIO26->SCLK
    GPIO_setPinConfig(GPIO_26_SPIB_CLK);
    GPIO_setDirectionMode(26, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(26, GPIO_PIN_TYPE_PULLUP);

    // GPIO27->SCS
    GPIO_setPinConfig(GPIO_27_SPIB_STE);
    GPIO_setDirectionMode(27, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(27, GPIO_PIN_TYPE_PULLUP);

    // GPIO28->SCIA_RX
    GPIO_setPinConfig(GPIO_28_SCIA_RX);
    GPIO_setDirectionMode(28, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(28, GPIO_PIN_TYPE_STD);

    // GPIO28->SCIA_TX
    GPIO_setPinConfig(GPIO_29_SCIA_TX);
    GPIO_setDirectionMode(29, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(29, GPIO_PIN_TYPE_STD);

    // GPIO30->CAN-RX
    GPIO_setPinConfig(GPIO_30_GPIO30);
    GPIO_setDirectionMode(30, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(30, GPIO_PIN_TYPE_STD);

    // GPIO31->LED1 on controlCard
    GPIO_setPinConfig(GPIO_31_GPIO31);
    GPIO_writePin(31, 1);
    GPIO_setDirectionMode(31, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(31, GPIO_PIN_TYPE_STD);

    // GPIO32->Reserve
    GPIO_setPinConfig(GPIO_32_GPIO32);
    GPIO_setDirectionMode(32, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(32, GPIO_PIN_TYPE_PULLUP);

    // GPIO33->Reserve
    GPIO_setPinConfig(GPIO_33_GPIO33);
    GPIO_setDirectionMode(33, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(33, GPIO_PIN_TYPE_STD);

    // LED2 on Control Card
    GPIO_setPinConfig(GPIO_34_GPIO34);
    GPIO_writePin(34, 1);
    GPIO_setDirectionMode(34, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(34, GPIO_PIN_TYPE_STD);

    // TDI (Remove R9)
    GPIO_setPinConfig(GPIO_35_TDI);

    // TDO (Remove R8)
    GPIO_setPinConfig(GPIO_37_TDO);

    // GPIO39->nFault
    GPIO_setPinConfig(GPIO_39_GPIO39);
    GPIO_setDirectionMode(39, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(39, GPIO_PIN_TYPE_STD);

    // QEPA
    GPIO_setPinConfig(GPIO_40_EQEP1A);
    GPIO_setDirectionMode(40, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(40, GPIO_PIN_TYPE_STD);

    // QEPB
    GPIO_setPinConfig(GPIO_41_EQEP1B);
    GPIO_setDirectionMode(41, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(41, GPIO_PIN_TYPE_STD);

    // GPIO42->LED
    GPIO_setPinConfig(GPIO_42_GPIO42);
    GPIO_writePin(42, 1);
    GPIO_setDirectionMode(42, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(42, GPIO_PIN_TYPE_STD);

    // GPIO43->Reserve
    GPIO_setPinConfig(GPIO_43_GPIO43);
    GPIO_setDirectionMode(43, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(43, GPIO_PIN_TYPE_STD);

    // GPIO44->Reserve
    GPIO_setPinConfig(GPIO_44_GPIO44);
    GPIO_setDirectionMode(44, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(44, GPIO_PIN_TYPE_STD);

    // GPIO45->Reserve
    GPIO_setPinConfig(GPIO_45_GPIO45);
    GPIO_setDirectionMode(45, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(45, GPIO_PIN_TYPE_STD);

    // GPIO46->Reserve
    GPIO_setPinConfig(GPIO_46_GPIO46);
    GPIO_setDirectionMode(46, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(46, GPIO_PIN_TYPE_STD);
    // end of DRV8301EVM_REVD
//------------------------------------------------------------------------------
#elif defined(DRV8312KIT_REVD)      // DRV8312KIT_REVD

    // GPIO0->EPWM1A->M1_UH
    GPIO_setPinConfig(GPIO_0_EPWM1_A);
    GPIO_setDirectionMode(0, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(0, GPIO_PIN_TYPE_STD);

#if !defined(MOTOR1_ISBLDC)
    // GPIO1->EPWM1B->M1_UL
    GPIO_setPinConfig(GPIO_1_EPWM1_B);
    GPIO_setDirectionMode(1, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(1, GPIO_PIN_TYPE_STD);
#else
    // GPIO1->DRV8312_RESETC
    GPIO_setPinConfig(GPIO_1_GPIO1);
    GPIO_writePin(1, 0);
    GPIO_setDirectionMode(1, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(1, GPIO_PIN_TYPE_PULLUP);
#endif  // MOTOR1_ISBLDC

    // GPIO2->EPWM2A->M1_VH
    GPIO_setPinConfig(GPIO_2_EPWM2_A);
    GPIO_setDirectionMode(2, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(2, GPIO_PIN_TYPE_STD);

#if !defined(MOTOR1_ISBLDC)
    // GPIO3->EPWM2B->M1_VL
    GPIO_setPinConfig(GPIO_3_EPWM2_B);
    GPIO_setDirectionMode(3, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(3, GPIO_PIN_TYPE_STD);
#else
    // GPIO3->DRV8312_RESETC
    GPIO_setPinConfig(GPIO_3_GPIO3);
    GPIO_writePin(3, 0);
    GPIO_setDirectionMode(3, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(3, GPIO_PIN_TYPE_PULLUP);
#endif  // MOTOR1_ISBLDC

    // GPIO4->EPWM3A->M1_WH
    GPIO_setPinConfig(GPIO_4_EPWM3_A);
    GPIO_setDirectionMode(4, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_STD);

#if !defined(MOTOR1_ISBLDC)
    // GPIO5->EPWM3B->M1_WL
    GPIO_setPinConfig(GPIO_5_EPWM3_B);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);
#else
    // GPIO5->DRV8312_RESETC
    GPIO_setPinConfig(GPIO_5_GPIO5);
    GPIO_writePin(5, 0);
    GPIO_setDirectionMode(5, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_PULLUP);
#endif  // MOTOR1_ISBLDC

    // GPIO6->Reserve
    GPIO_setPinConfig(GPIO_6_GPIO6);
    GPIO_setDirectionMode(6, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(6, GPIO_PIN_TYPE_STD);

#ifdef SST_ENABLE
    // GPIO7->EPWM4B for SST
    GPIO_setPinConfig(GPIO_7_EPWM4_B);
    GPIO_setDirectionMode(7, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(7, GPIO_PIN_TYPE_STD);
#else  // !SST_ENABLE
  // GPIO7->STOP
  GPIO_setPinConfig(GPIO_7_GPIO7);
  GPIO_setDirectionMode(7, GPIO_DIR_MODE_IN);
  GPIO_setPadConfig(7, GPIO_PIN_TYPE_PULLUP);
#endif  // !SST_ENABLE

    // PWM5A->PWMDAC3
    GPIO_setPinConfig(GPIO_8_EPWM5_A);
    GPIO_setDirectionMode(8, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(8, GPIO_PIN_TYPE_STD);

    // GPIO09->START
    GPIO_setPinConfig(GPIO_9_GPIO9);
    GPIO_setDirectionMode(9, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(9, GPIO_PIN_TYPE_PULLUP);

    // PWM6A->PWMDAC1
    GPIO_setPinConfig(GPIO_10_EPWM6_A);
    GPIO_setDirectionMode(10, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(10, GPIO_PIN_TYPE_STD);

    // PWM6B->PWMDAC2
    GPIO_setPinConfig(GPIO_11_EPWM6_B);
    GPIO_setDirectionMode(11, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(11, GPIO_PIN_TYPE_PULLUP);

    // GPIO12->EPWM7A
    GPIO_setPinConfig(GPIO_12_EPWM7_A);
    GPIO_setDirectionMode(12, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(12, GPIO_PIN_TYPE_STD);

    // GPIO13->Reserve
    GPIO_setPinConfig(GPIO_13_GPIO13);
    GPIO_setDirectionMode(13, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(13, GPIO_PIN_TYPE_STD);

    // GPIO14->Reserve
    GPIO_setPinConfig(GPIO_14_GPIO14);
    GPIO_setDirectionMode(14, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(14, GPIO_PIN_TYPE_STD);

    // GPIO15->Reserve
    GPIO_setPinConfig(GPIO_15_GPIO15);
    GPIO_setDirectionMode(15, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(15, GPIO_PIN_TYPE_STD);

    // GPIO16->Reserve
    GPIO_setPinConfig(GPIO_16_GPIO16);
    GPIO_setDirectionMode(16, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(16, GPIO_PIN_TYPE_STD);

    // GPIO17->Reserve
    GPIO_setPinConfig(GPIO_17_GPIO17);
    GPIO_setDirectionMode(17, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(17, GPIO_PIN_TYPE_STD);

    // GPIO18->Reserve
    GPIO_setPinConfig(GPIO_18_GPIO18);
    GPIO_setDirectionMode(18, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(18, GPIO_PIN_TYPE_STD);

    // GPIO19->Reserve
    GPIO_setPinConfig(GPIO_19_GPIO19);
    GPIO_setDirectionMode(19, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(19, GPIO_PIN_TYPE_STD);

    // GPIO20->EQEP1_A*
    GPIO_setPinConfig(GPIO_20_EQEP1_A);
    GPIO_setDirectionMode(20, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(20, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(20, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(20, 2);

    // GPIO21->EQEP1_B*
    GPIO_setPinConfig(GPIO_21_EQEP1_B);
    GPIO_setDirectionMode(21, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(21, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(21, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(21, 2);

    // GPIO22->STATUS*
    GPIO_setPinConfig(GPIO_22_GPIO22);
    GPIO_writePin(22, 1);
    GPIO_setDirectionMode(22, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(22, GPIO_PIN_TYPE_PULLUP);

    // GPIO23->EQEP1_INDEX*
    GPIO_setPinConfig(GPIO_23_EQEP1_INDEX);
    GPIO_setDirectionMode(23, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(23, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(23, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(23, 2);

    // GPIO24->HALL_U*
    GPIO_setPinConfig(GPIO_24_GPIO24);
    GPIO_setDirectionMode(24, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(24, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(24, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(24, 2);

    // GPIO25->HALL_V*
    GPIO_setPinConfig(GPIO_25_GPIO25);
    GPIO_setDirectionMode(25, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(25, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(25, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(25, 2);

    // GPIO26->HALL_W*
    GPIO_setPinConfig(GPIO_26_GPIO26);
    GPIO_setDirectionMode(26, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(26, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(26, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(26, 2);

    // GPIO27->M1_ISR
    GPIO_setPinConfig(GPIO_27_GPIO27);
    GPIO_setDirectionMode(27, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(27, GPIO_PIN_TYPE_STD);

    // GPIO28->SCIA_RX*
    GPIO_setPinConfig(GPIO_28_SCIA_RX);
    GPIO_setDirectionMode(28, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(28, GPIO_PIN_TYPE_STD);

    // GPIO29->SCIA_TX*
    GPIO_setPinConfig(GPIO_29_SCIA_TX);
    GPIO_setDirectionMode(29, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(29, GPIO_PIN_TYPE_STD);

    // GPIO30->CANRX
    GPIO_setPinConfig(GPIO_30_CANA_RX);
    GPIO_setDirectionMode(30, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(30, GPIO_PIN_TYPE_STD);

    // LED1 on controlCard
    GPIO_setPinConfig(GPIO_31_GPIO31);
    GPIO_writePin(33, 1);
    GPIO_setDirectionMode(31, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(31, GPIO_PIN_TYPE_STD);

    // GPIO32->Reserve
    GPIO_setPinConfig(GPIO_32_GPIO32);
    GPIO_setDirectionMode(32, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(32, GPIO_PIN_TYPE_STD);

    // GPIO33->Reserve
    GPIO_setPinConfig(GPIO_33_GPIO33);
    GPIO_setDirectionMode(33, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(33, GPIO_PIN_TYPE_STD);

    // LED2 on Control Card
    GPIO_setPinConfig(GPIO_34_GPIO34);
    GPIO_writePin(34, 1);
    GPIO_setDirectionMode(34, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(34, GPIO_PIN_TYPE_STD);

    // TDI
    GPIO_setPinConfig(GPIO_35_TDI);

    // TDO
    GPIO_setPinConfig(GPIO_37_TDO);

    // GPIO39->Reserve
    GPIO_setPinConfig(GPIO_39_GPIO39);
    GPIO_setDirectionMode(39, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(39, GPIO_PIN_TYPE_STD);

    // GPIO40->LED1 on DRV8312 Board
    GPIO_setPinConfig(GPIO_40_GPIO40);
    GPIO_setDirectionMode(40, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(40, GPIO_PIN_TYPE_STD);

    // GPIO41->nOCTW
    GPIO_setPinConfig(GPIO_41_GPIO41);
    GPIO_setDirectionMode(41, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(41, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(41, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(41, 2);

    // GPIO42->Reserve
    GPIO_setPinConfig(GPIO_42_GPIO42);
    GPIO_setDirectionMode(42, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(42, GPIO_PIN_TYPE_STD);

    // GPIO43->Reserve
    GPIO_setPinConfig(GPIO_43_GPIO43);
    GPIO_setDirectionMode(43, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(43, GPIO_PIN_TYPE_STD);

    // GPIO44->Reserve
    GPIO_setPinConfig(GPIO_44_GPIO44);
    GPIO_setDirectionMode(44, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(44, GPIO_PIN_TYPE_STD);

    // GPIO45->Reserve
    GPIO_setPinConfig(GPIO_45_GPIO45);
    GPIO_setDirectionMode(45, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(45, GPIO_PIN_TYPE_STD);

    // GPIO46->Reserve
    GPIO_setPinConfig(GPIO_46_GPIO46);
    GPIO_setDirectionMode(46, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(46, GPIO_PIN_TYPE_STD);

    // GPIO47->nFault
    GPIO_setPinConfig(GPIO_47_GPIO47);
    GPIO_setDirectionMode(47, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(47, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(47, GPIO_QUAL_3SAMPLE);
    GPIO_setQualificationPeriod(47, 2);

    // GPIO59->LED2 on DRV8312 Board
    GPIO_setPinConfig(GPIO_59_GPIO59);
    GPIO_setDirectionMode(59, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(59, GPIO_PIN_TYPE_STD);
    // end of DRV8312KIT_REVD
#else
#error The GPIOs is not configured for motor control board
#endif

    return;
}  // end of HAL_setupGPIOs() function

void HAL_setupPWMs(HAL_MTR_Handle handle)
{
    HAL_MTR_Obj    *obj = (HAL_MTR_Obj *)handle;
    uint16_t  cnt;

    uint16_t pwmPeriodCycles = (uint16_t)(USER_M1_PWM_TBPRD_NUM);
    uint16_t numPWMTicksPerISRTick = USER_M1_NUM_PWM_TICKS_PER_ISR_TICK;

    // disable the ePWM module time base clock sync signal
    // to synchronize all of the PWMs
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    // turns off the outputs of the EPWM peripherals which will put the power
    // switches into a high impedance state.
    EPWM_forceTripZoneEvent(obj->pwmHandle[0], EPWM_TZ_FORCE_EVENT_OST);
    EPWM_forceTripZoneEvent(obj->pwmHandle[1], EPWM_TZ_FORCE_EVENT_OST);
    EPWM_forceTripZoneEvent(obj->pwmHandle[2], EPWM_TZ_FORCE_EVENT_OST);
     EPWM_forceTripZoneEvent(obj->pwmHandle[3], EPWM_TZ_FORCE_EVENT_OST);//syh

#if defined(BSXL8323RS_REVA) || defined(BSXL8323RH_REVB) || \
    defined(BSXL8353RS_REVA) || defined(BSXL8316RT_REVA) || \
    defined(BSXL3PHGAN_REVA) || defined(HVMTRPFC_REV1P1) || \
    defined(DRV8329AEVM_REVA)

    for(cnt=0; cnt<4; cnt++)
    {
        // setup the Time-Base Control Register (TBCTL)
        EPWM_setTimeBaseCounterMode(obj->pwmHandle[cnt],
                                    EPWM_COUNTER_MODE_UP_DOWN);

        EPWM_disablePhaseShiftLoad(obj->pwmHandle[cnt]);

        EPWM_setPeriodLoadMode(obj->pwmHandle[cnt], EPWM_PERIOD_DIRECT_LOAD);

        EPWM_enableSyncOutPulseSource(obj->pwmHandle[cnt],
                                      EPWM_SYNC_OUT_PULSE_ON_SOFTWARE);

        EPWM_setClockPrescaler(obj->pwmHandle[cnt], EPWM_CLOCK_DIVIDER_1,
                                 EPWM_HSCLOCK_DIVIDER_1);

        EPWM_setCountModeAfterSync(obj->pwmHandle[cnt],
                                   EPWM_COUNT_MODE_UP_AFTER_SYNC);

        EPWM_setEmulationMode(obj->pwmHandle[cnt], EPWM_EMULATION_FREE_RUN);

        // setup the Timer-Based Phase Register (TBPHS)
        EPWM_setPhaseShift(obj->pwmHandle[cnt], 0);

        // setup the Time-Base Counter Register (TBCTR)
        EPWM_setTimeBaseCounter(obj->pwmHandle[cnt], 0);

        // setup the Time-Base Period Register (TBPRD)
        // set to zero initially
        EPWM_setTimeBasePeriod(obj->pwmHandle[cnt], 0);

        // setup the Counter-Compare Control Register (CMPCTL)
        EPWM_setCounterCompareShadowLoadMode(obj->pwmHandle[cnt],
                                             EPWM_COUNTER_COMPARE_A,
                                             EPWM_COMP_LOAD_ON_CNTR_ZERO);

        EPWM_setCounterCompareShadowLoadMode(obj->pwmHandle[cnt],
                                             EPWM_COUNTER_COMPARE_B,
                                             EPWM_COMP_LOAD_ON_CNTR_ZERO);

        EPWM_setCounterCompareShadowLoadMode(obj->pwmHandle[cnt],
                                             EPWM_COUNTER_COMPARE_C,
                                             EPWM_COMP_LOAD_ON_CNTR_ZERO);

        EPWM_setCounterCompareShadowLoadMode(obj->pwmHandle[cnt],
                                             EPWM_COUNTER_COMPARE_D,
                                             EPWM_COMP_LOAD_ON_CNTR_ZERO);

#if defined(MOTOR1_ISBLDC)
#if defined(MOTOR1_DUALPWM)
        // setup the Action-Qualifier Output A Register (AQCTLA)
        EPWM_setActionQualifierAction(obj->pwmHandle[cnt],
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_HIGH,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);

        EPWM_setActionQualifierAction(obj->pwmHandle[cnt],
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_HIGH,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);

        EPWM_setActionQualifierAction(obj->pwmHandle[cnt],
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_LOW,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);

        EPWM_setActionQualifierAction(obj->pwmHandle[cnt],
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_LOW,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);

        // setup the Dead-Band Generator Control Register (DBCTL)
        EPWM_setDeadBandDelayMode(obj->pwmHandle[cnt], EPWM_DB_RED, true);
        EPWM_setDeadBandDelayMode(obj->pwmHandle[cnt], EPWM_DB_FED, true);

        // select EPWMA as the input to the dead band generator
        EPWM_setRisingEdgeDeadBandDelayInput(obj->pwmHandle[cnt],
                                             EPWM_DB_INPUT_EPWMA);

        // configure the right polarity for active high complementary config.
        EPWM_setDeadBandDelayPolarity(obj->pwmHandle[cnt],
                                      EPWM_DB_RED,
                                      EPWM_DB_POLARITY_ACTIVE_HIGH);

        EPWM_setDeadBandDelayPolarity(obj->pwmHandle[cnt],
                                      EPWM_DB_FED,
                                      EPWM_DB_POLARITY_ACTIVE_LOW);

        // setup the Dead-Band Rising Edge Delay Register (DBRED)
        EPWM_setRisingEdgeDelayCount(obj->pwmHandle[cnt], MTR1_PWM_DBRED_CNT);

        // setup the Dead-Band Falling Edge Delay Register (DBFED)
        EPWM_setFallingEdgeDelayCount(obj->pwmHandle[cnt], MTR1_PWM_DBFED_CNT);
#else   // !MOTOR1_DUALPWM
    // setup the Action-Qualifier Output A Register (AQCTLA)
    EPWM_setActionQualifierAction(obj->pwmHandle[cnt],
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);

    EPWM_setActionQualifierAction(obj->pwmHandle[cnt],
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);

    // setup the Action-qualifier Continuous Software Force Register (AQCSFRC)
    EPWM_setActionQualifierContSWForceAction(obj->pwmHandle[cnt],
                                             EPWM_AQ_OUTPUT_B,
                                             EPWM_AQ_SW_OUTPUT_LOW);

    // setup the Dead-Band Generator Control Register (DBCTL)
    EPWM_setDeadBandDelayMode(obj->pwmHandle[cnt], EPWM_DB_RED, false);
    EPWM_setDeadBandDelayMode(obj->pwmHandle[cnt], EPWM_DB_FED, false);
#endif  // !MOTOR1_DUALPWM
#else   //!MOTOR1_ISBLDC
#if defined(MOTOR1_DCLINKSS)
        // setup the Action-Qualifier Output A Register (AQCTLA)
        EPWM_setActionQualifierAction(obj->pwmHandle[cnt],
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_HIGH,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);

        EPWM_setActionQualifierAction(obj->pwmHandle[cnt],
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_LOW,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);

        EPWM_setActionQualifierAction(obj->pwmHandle[cnt],
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_LOW,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);

        EPWM_setActionQualifierAction(obj->pwmHandle[cnt],
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_HIGH,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);
#else  // !(MOTOR1_DCLINKSS)
        // setup the Action-Qualifier Output A Register (AQCTLA)
        EPWM_setActionQualifierAction(obj->pwmHandle[cnt],
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_HIGH,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);

        EPWM_setActionQualifierAction(obj->pwmHandle[cnt],
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_HIGH,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);

        EPWM_setActionQualifierAction(obj->pwmHandle[cnt],
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_LOW,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);

        EPWM_setActionQualifierAction(obj->pwmHandle[cnt],
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_LOW,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);

#endif  // !(MOTOR1_DCLINKSS)

        // setup the Dead-Band Generator Control Register (DBCTL)
        EPWM_setDeadBandDelayMode(obj->pwmHandle[cnt], EPWM_DB_RED, true);
        EPWM_setDeadBandDelayMode(obj->pwmHandle[cnt], EPWM_DB_FED, true);

        // select EPWMA as the input to the dead band generator
        EPWM_setRisingEdgeDeadBandDelayInput(obj->pwmHandle[cnt],
                                             EPWM_DB_INPUT_EPWMA);

        // configure the right polarity for active high complementary config.
        EPWM_setDeadBandDelayPolarity(obj->pwmHandle[cnt],
                                      EPWM_DB_RED,
                                      EPWM_DB_POLARITY_ACTIVE_HIGH);
        EPWM_setDeadBandDelayPolarity(obj->pwmHandle[cnt],
                                      EPWM_DB_FED,
                                      EPWM_DB_POLARITY_ACTIVE_LOW);

        // setup the Dead-Band Rising Edge Delay Register (DBRED)
        EPWM_setRisingEdgeDelayCount(obj->pwmHandle[cnt], MTR1_PWM_DBRED_CNT);

        // setup the Dead-Band Falling Edge Delay Register (DBFED)
        EPWM_setFallingEdgeDelayCount(obj->pwmHandle[cnt], MTR1_PWM_DBFED_CNT);

#endif  //!MOTOR1_ISBLDC

        // setup the PWM-Chopper Control Register (PCCTL)
        EPWM_disableChopper(obj->pwmHandle[cnt]);

        // setup the Trip Zone Select Register (TZSEL)
        EPWM_disableTripZoneSignals(obj->pwmHandle[cnt], HAL_TZSEL_SIGNALS_ALL);
    }

    // BSXL8323RS_REVA || BSXL8323RH_REVB || BSXL8353RS_REVA || \
    // BSXL8316RT_REVA || BSXL3PHGAN_REVA || HVMTRPFC_REV1P1 || \
    // DRV8329AEVM_REVA
#else
#error The PWM is not configured for motor_1 control
#endif  // boards

#if defined(MOTOR1_ISBLDC)
    // setup the Event Trigger Selection Register (ETSEL)
    EPWM_setInterruptSource(obj->pwmHandle[0], EPWM_INT_TBCTR_ZERO);

    EPWM_disableInterrupt(obj->pwmHandle[0]);

    EPWM_setADCTriggerSource(obj->pwmHandle[0],
                             EPWM_SOC_A, EPWM_SOC_TBCTR_ZERO);

    EPWM_enableADCTrigger(obj->pwmHandle[0], EPWM_SOC_A);

    EPWM_setADCTriggerSource(obj->pwmHandle[0],
                              EPWM_SOC_B, EPWM_SOC_TBCTR_U_CMPB);

    EPWM_enableADCTrigger(obj->pwmHandle[0], EPWM_SOC_B);
#elif defined(MOTOR1_DCLINKSS)
    // setup the Event Trigger Selection Register (ETSEL)
    EPWM_setInterruptSource(obj->pwmHandle[0], EPWM_INT_TBCTR_ZERO);

    EPWM_enableInterrupt(obj->pwmHandle[0]);

    EPWM_setADCTriggerSource(obj->pwmHandle[0],
                             EPWM_SOC_A, EPWM_SOC_TBCTR_D_CMPC);

    EPWM_enableADCTrigger(obj->pwmHandle[0], EPWM_SOC_A);

    // ADC SOC trigger for the 1st dc-link current sampling
    EPWM_setADCTriggerSource(obj->pwmHandle[1],
                                 EPWM_SOC_A,
                                 EPWM_SOC_TBCTR_U_CMPC);

    EPWM_enableADCTrigger(obj->pwmHandle[1], EPWM_SOC_A);

    // ADC SOC trigger for the 2nd dc-link current sampling
    EPWM_setADCTriggerSource(obj->pwmHandle[1],
                             EPWM_SOC_B,
                             EPWM_SOC_TBCTR_U_CMPD);

    EPWM_enableADCTrigger(obj->pwmHandle[1], EPWM_SOC_B);

    // ADC SOC trigger for the 3rd dc-link current sampling
    EPWM_setADCTriggerSource(obj->pwmHandle[2],
                                 EPWM_SOC_A,
                                 EPWM_SOC_TBCTR_D_CMPC);

    EPWM_enableADCTrigger(obj->pwmHandle[2], EPWM_SOC_A);

    // ADC SOC trigger for the 4th dc-link current sampling
    EPWM_setADCTriggerSource(obj->pwmHandle[2],
                             EPWM_SOC_B,
                             EPWM_SOC_TBCTR_D_CMPD);

    EPWM_enableADCTrigger(obj->pwmHandle[2], EPWM_SOC_B);
#else   //!(MOTOR1_ISBLDC || MOTOR1_DCLINKSS)
    // setup the Event Trigger Selection Register (ETSEL)
    EPWM_setInterruptSource(obj->pwmHandle[0], EPWM_INT_TBCTR_ZERO);

    EPWM_enableInterrupt(obj->pwmHandle[0]);

    EPWM_setADCTriggerSource(obj->pwmHandle[0],
                             EPWM_SOC_A, EPWM_SOC_TBCTR_D_CMPC);

    EPWM_enableADCTrigger(obj->pwmHandle[0], EPWM_SOC_A);
#endif  // !(MOTOR1_ISBLDC || MOTOR1_DCLINKSS)

    // setup the Event Trigger Prescale Register (ETPS)
    if(numPWMTicksPerISRTick > 15)
    {
        numPWMTicksPerISRTick = 15;
    }
    else if(numPWMTicksPerISRTick < 1)
    {
        numPWMTicksPerISRTick = 1;
    }

    EPWM_setInterruptEventCount(obj->pwmHandle[0], numPWMTicksPerISRTick);

    EPWM_setADCTriggerEventPrescale(obj->pwmHandle[0], EPWM_SOC_A,
                                    numPWMTicksPerISRTick);
    EPWM_setADCTriggerEventPrescale(obj->pwmHandle[0], EPWM_SOC_B,
                                    numPWMTicksPerISRTick);

#if defined(MOTOR1_DCLINKSS)
    EPWM_setADCTriggerEventPrescale(obj->pwmHandle[1], EPWM_SOC_A,
                                    numPWMTicksPerISRTick);
    EPWM_setADCTriggerEventPrescale(obj->pwmHandle[1], EPWM_SOC_B,
                                    numPWMTicksPerISRTick);

    EPWM_setADCTriggerEventPrescale(obj->pwmHandle[2], EPWM_SOC_A,
                                    numPWMTicksPerISRTick);
    EPWM_setADCTriggerEventPrescale(obj->pwmHandle[2], EPWM_SOC_B,
                                    numPWMTicksPerISRTick);
#endif  //MOTOR1_DCLINKSS

    // setup the Event Trigger Clear Register (ETCLR)
    EPWM_clearEventTriggerInterruptFlag(obj->pwmHandle[0]);
    EPWM_clearADCTriggerFlag(obj->pwmHandle[0], EPWM_SOC_A);
    EPWM_clearADCTriggerFlag(obj->pwmHandle[0], EPWM_SOC_B);

    // since the PWM is configured as an up/down counter, the period register is
    // set to one-half of the desired PWM period
    EPWM_setTimeBasePeriod(obj->pwmHandle[0], pwmPeriodCycles);
    EPWM_setTimeBasePeriod(obj->pwmHandle[1], pwmPeriodCycles);
    EPWM_setTimeBasePeriod(obj->pwmHandle[2], pwmPeriodCycles);
     EPWM_setTimeBasePeriod(obj->pwmHandle[3], pwmPeriodCycles);

    // write the PWM data value  for ADC trigger
    EPWM_setCounterCompareValue(obj->pwmHandle[0], EPWM_COUNTER_COMPARE_C, 10);

    // sync "down-stream"
    EPWM_enableSyncOutPulseSource(obj->pwmHandle[0], EPWM_SYNC_OUT_PULSE_ON_CNTR_ZERO);

    // write the PWM data value  for ADC trigger
#if defined(MOTOR1_DCLINKSS)
    EPWM_clearADCTriggerFlag(obj->pwmHandle[1], EPWM_SOC_A);
    EPWM_clearADCTriggerFlag(obj->pwmHandle[1], EPWM_SOC_B);

    EPWM_clearADCTriggerFlag(obj->pwmHandle[2], EPWM_SOC_A);
    EPWM_clearADCTriggerFlag(obj->pwmHandle[2], EPWM_SOC_B);

    EPWM_setCounterCompareValue(obj->pwmHandle[1],
                                EPWM_COUNTER_COMPARE_C, pwmPeriodCycles>>1);
    EPWM_setCounterCompareValue(obj->pwmHandle[1],
                                EPWM_COUNTER_COMPARE_D, pwmPeriodCycles>>1);

    EPWM_setCounterCompareValue(obj->pwmHandle[2],
                                EPWM_COUNTER_COMPARE_C, pwmPeriodCycles>>1);
    EPWM_setCounterCompareValue(obj->pwmHandle[2],
                                EPWM_COUNTER_COMPARE_D, pwmPeriodCycles>>1);

#endif  //MOTOR1_DCLINKSS


    return;
}  // end of HAL_setupPWMs() function

#if defined(EPWMDAC_MODE)
void HAL_setupPWMDACs(HAL_Handle handle,
                   const float32_t systemFreq_MHz)
{
    HAL_Obj   *obj = (HAL_Obj *)handle;

    // PWMDAC frequency = 100kHz, calculate the period for pwm
    uint16_t  period_cycles = (uint16_t)(systemFreq_MHz *
                                  (float32_t)(1000.0f / HA_PWMDAC_FREQ_KHZ));
    uint16_t  cnt;

    for(cnt = 0; cnt < 4; cnt++)
    {
        // setup the Time-Base Control Register (TBCTL)
        EPWM_setTimeBaseCounterMode(obj->pwmDACHandle[cnt],
                                    EPWM_COUNTER_MODE_UP);

        EPWM_disablePhaseShiftLoad(obj->pwmDACHandle[cnt]);

        EPWM_setPeriodLoadMode(obj->pwmDACHandle[cnt], EPWM_PERIOD_DIRECT_LOAD);

        EPWM_enableSyncOutPulseSource(obj->pwmDACHandle[cnt],
                                      EPWM_SYNC_OUT_PULSE_ON_SOFTWARE);

        EPWM_setClockPrescaler(obj->pwmDACHandle[cnt], EPWM_CLOCK_DIVIDER_1,
                                 EPWM_HSCLOCK_DIVIDER_1);

        EPWM_setCountModeAfterSync(obj->pwmDACHandle[cnt],
                                   EPWM_COUNT_MODE_UP_AFTER_SYNC);

        EPWM_setEmulationMode(obj->pwmDACHandle[cnt], EPWM_EMULATION_FREE_RUN);

        // setup the Timer-Based Phase Register (TBPHS)
        EPWM_setPhaseShift(obj->pwmDACHandle[cnt], 0);

        // setup the Time-Base Counter Register (TBCTR)
        EPWM_setTimeBaseCounter(obj->pwmDACHandle[cnt], 0);

        // setup the Time-Base Period Register (TBPRD)
        // set to zero initially
        EPWM_setTimeBasePeriod(obj->pwmDACHandle[cnt], 0);

        // setup the Counter-Compare Control Register (CMPCTL)
        EPWM_setCounterCompareShadowLoadMode(obj->pwmDACHandle[cnt],
                                             EPWM_COUNTER_COMPARE_A,
                                             EPWM_COMP_LOAD_ON_CNTR_ZERO);

        // setup the Action-Qualifier Output A Register (AQCTLA)
        EPWM_setActionQualifierAction(obj->pwmDACHandle[cnt],
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_LOW,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);

        EPWM_setActionQualifierAction(obj->pwmDACHandle[cnt],
                                      EPWM_AQ_OUTPUT_A,
                                      EPWM_AQ_OUTPUT_HIGH,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);

        // setup the Action-Qualifier Output B Register (AQCTLB)
        EPWM_setActionQualifierAction(obj->pwmDACHandle[cnt],
                                      EPWM_AQ_OUTPUT_B,
                                      EPWM_AQ_OUTPUT_LOW,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);

        EPWM_setActionQualifierAction(obj->pwmDACHandle[cnt],
                                      EPWM_AQ_OUTPUT_B,
                                      EPWM_AQ_OUTPUT_HIGH,
                                      EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);


        // setup the Dead-Band Generator Control Register (DBCTL)
        EPWM_setDeadBandDelayMode(obj->pwmDACHandle[cnt], EPWM_DB_RED, false);

        EPWM_setDeadBandDelayMode(obj->pwmDACHandle[cnt], EPWM_DB_FED, false);

        // setup the PWM-Chopper Control Register (PCCTL)
        EPWM_disableChopper(obj->pwmDACHandle[cnt]);

        // setup the Trip Zone Select Register (TZSEL)
        EPWM_disableTripZoneSignals(obj->pwmDACHandle[cnt], HAL_TZSEL_SIGNALS_ALL);

        // since the PWM is configured as an up/down counter, the period
        // register is set to one-half of the desired PWM period
        EPWM_setTimeBasePeriod(obj->pwmDACHandle[cnt], period_cycles);
    }

    return;
}  // end of HAL_setupPWMDACs() function
#endif  // EPWMDAC_MODE

#if defined(MOTOR1_ENC)
void HAL_setupQEP(HAL_MTR_Handle handle)
{
    HAL_MTR_Obj   *obj = (HAL_MTR_Obj *)handle;

    // Configure the decoder for quadrature count mode, counting both
    // rising and falling edges (that is, 2x resolution), QDECCTL
    EQEP_setDecoderConfig(obj->qepHandle, (EQEP_CONFIG_2X_RESOLUTION |
                                           EQEP_CONFIG_QUADRATURE |
                                           EQEP_CONFIG_NO_SWAP) );

    EQEP_setEmulationMode(obj->qepHandle, EQEP_EMULATIONMODE_RUNFREE);

    // Configure the position counter to be latched on a unit time out
    // and latch on rising edge of index pulse
    EQEP_setLatchMode(obj->qepHandle, (EQEP_LATCH_RISING_INDEX |
                                       EQEP_LATCH_UNIT_TIME_OUT) );

    EQEP_setPositionCounterConfig(obj->qepHandle, EQEP_POSITION_RESET_MAX_POS,
                                  (uint32_t)(USER_MOTOR1_ENC_POS_MAX));

#if defined(ENC_UVW)
    EQEP_setInitialPosition(obj->qepHandle, USER_MOTOR1_ENC_POS_OFFSET);

    EQEP_setPositionInitMode(obj->qepHandle, EQEP_INIT_RISING_INDEX);
#endif

    // Enable the unit timer, setting the frequency to 10KHz
    // QUPRD, QEPCTL
    EQEP_enableUnitTimer(obj->qepHandle, (USER_M1_QEP_UNIT_TIMER_TICKS - 1));

    // Disables the eQEP module position-compare unit
    EQEP_disableCompare(obj->qepHandle);

    // Configure and enable the edge-capture unit. The capture clock divider is
    // SYSCLKOUT/128. The unit-position event divider is QCLK/32.
    EQEP_setCaptureConfig(obj->qepHandle, EQEP_CAPTURE_CLK_DIV_128,
                                          EQEP_UNIT_POS_EVNT_DIV_32);

    // Enable QEP edge-capture unit
    EQEP_enableCapture(obj->qepHandle);

    // Enable UTO on QEP
    EQEP_enableInterrupt(obj->qepHandle, EQEP_INT_UNIT_TIME_OUT);

    // Enable the eQEP module
    EQEP_enableModule(obj->qepHandle);

    return;
}
#endif  // MOTOR1_ENC

void HAL_setupSCIA(HAL_Handle halHandle)
{
        HAL_Obj *obj = (HAL_Obj *)halHandle;
    
    // SCIB 모듈 설정 (obj->sciHandle = SCIB_BASE)
    SCI_performSoftwareReset(obj->sciHandle);
    SCI_setConfig(obj->sciHandle, DEVICE_LSPCLK_FREQ, 57600,
                  (SCI_CONFIG_WLEN_8 | SCI_CONFIG_STOP_ONE | SCI_CONFIG_PAR_NONE));
    SCI_resetChannels(obj->sciHandle);
    SCI_resetRxFIFO(obj->sciHandle);
    SCI_resetTxFIFO(obj->sciHandle);
    SCI_clearInterruptStatus(obj->sciHandle, SCI_INT_TXFF | SCI_INT_RXFF);
    SCI_enableFIFO(obj->sciHandle);
    SCI_enableModule(obj->sciHandle);
    SCI_performSoftwareReset(obj->sciHandle);
    
    
    /*HAL_Obj *obj = (HAL_Obj *)halHandle;

    // Initialize SCIA and its FIFO.
    SCI_performSoftwareReset(obj->sciHandle);

    // Configure SCIA for echoback.  // test sfra 9600 -> 57600
    SCI_setConfig(obj->sciHandle, DEVICE_LSPCLK_FREQ, 57600,
                        ( SCI_CONFIG_WLEN_8 |
                          SCI_CONFIG_STOP_ONE |
                          SCI_CONFIG_PAR_NONE ) );

    SCI_resetChannels(obj->sciHandle);

    SCI_resetRxFIFO(obj->sciHandle);

    SCI_resetTxFIFO(obj->sciHandle);

    SCI_clearInterruptStatus(obj->sciHandle, SCI_INT_TXFF | SCI_INT_RXFF);

    SCI_enableFIFO(obj->sciHandle);

    SCI_enableModule(obj->sciHandle);

    SCI_performSoftwareReset(obj->sciHandle);*/
/* test code */
    // SCIB GPIO 설정
/*GPIO_setPinConfig(GPIO_15_SCIB_RX);
GPIO_setDirectionMode(15, GPIO_DIR_MODE_IN);
GPIO_setQualificationMode(15, GPIO_QUAL_ASYNC);

GPIO_setPinConfig(GPIO_56_SCIB_TX);
GPIO_setDirectionMode(56, GPIO_DIR_MODE_OUT);
GPIO_setQualificationMode(56, GPIO_QUAL_ASYNC);

// SCIB 모듈 설정
SCI_setConfig(SCIB_BASE,
              DEVICE_LSPCLK_FREQ,
              57600,
              (SCI_CONFIG_WLEN_8 |
               SCI_CONFIG_STOP_ONE |
               SCI_CONFIG_PAR_NONE));
SCI_enableModule(SCIB_BASE);
SCI_performSoftwareReset(SCIB_BASE);*/
/* test code */
    return;
}  // end of DRV_setupSci() function

void HAL_setupI2CA(HAL_Handle halHandle) //i2c 수정
{
    HAL_Obj *obj = (HAL_Obj *)halHandle;

    // Must put I2C into reset before configuring it
    I2C_disableModule(obj->i2cHandle);

    // I2C configuration. Use a 400kHz I2CCLK with a 50% duty cycle.
    I2C_initMaster(obj->i2cHandle, DEVICE_SYSCLK_FREQ, 400000, I2C_DUTYCYCLE_50);
    I2C_setConfig(obj->i2cHandle, I2C_MASTER_SEND_MODE);
    I2C_setSlaveAddress(obj->i2cHandle, I2C_SLAVE_ADDRESS);
    I2C_disableLoopback(obj->i2cHandle);
    I2C_setBitCount(obj->i2cHandle, I2C_BITCOUNT_8);
    I2C_setDataCount(obj->i2cHandle, 2);
    I2C_setAddressMode(obj->i2cHandle, I2C_ADDR_MODE_7BITS);

    // Enable stop condition and register-access-ready interrupts
    /* test del. 1112
    I2C_enableInterrupt(obj->i2cHandle, I2C_INT_ADDR_SLAVE |
                                        I2C_INT_ARB_LOST |
                                        I2C_INT_NO_ACK |
                                        I2C_INT_STOP_CONDITION);
     */
    // FIFO configuration
    I2C_enableFIFO(obj->i2cHandle);
    I2C_setFIFOInterruptLevel(obj->i2cHandle, I2C_FIFO_TXEMPTY, I2C_FIFO_RX2);

//    I2C_clearInterruptStatus(obj->i2cHandle, I2C_INT_RXFF | I2C_INT_TXFF);
    I2C_clearInterruptStatus(obj->i2cHandle, I2C_INT_ARB_LOST | I2C_INT_NO_ACK);

    // Configuration complete. Enable the module.
    I2C_setEmulationMode(obj->i2cHandle, I2C_EMULATION_FREE_RUN);
    I2C_enableModule(obj->i2cHandle);

    return;
}  // end of HAL_setupI2CA() function

void HAL_setupTimeBaseTimer(HAL_Handle handle, const float32_t timeBaseFreq_Hz)
{
    HAL_Obj  *obj = (HAL_Obj *)handle;

    uint32_t timerPeriod = (uint32_t)((USER_SYSTEM_FREQ_MHz * 1000.0f *1000.0f) /
                                      timeBaseFreq_Hz);

    // use timer 0 for CPU usage diagnostics
    CPUTimer_setPreScaler(obj->timerHandle[0], 0);

    CPUTimer_setEmulationMode(obj->timerHandle[0],
                              CPUTIMER_EMULATIONMODE_RUNFREE);

    CPUTimer_setPeriod(obj->timerHandle[0], timerPeriod);

    CPUTimer_startTimer(obj->timerHandle[0]);

    return;
}  // end of HAL_setupTimeBaseTimer() function

void HAL_setupADCTriggerTimer(HAL_Handle handle, const float32_t adcTriggerFreq_Hz)
{
    HAL_Obj  *obj = (HAL_Obj *)handle;

    uint32_t timerPeriod = (uint32_t)((USER_SYSTEM_FREQ_MHz * 1000.0f *1000.0f) /
                                      adcTriggerFreq_Hz);

    // use timer 1 for ADC trigger
    CPUTimer_setPreScaler(obj->timerHandle[1], 0);

    CPUTimer_setEmulationMode(obj->timerHandle[1],
                              CPUTIMER_EMULATIONMODE_RUNFREE);

    CPUTimer_setPeriod(obj->timerHandle[1], timerPeriod);

    CPUTimer_enableInterrupt(obj->timerHandle[1]);

    CPUTimer_startTimer(obj->timerHandle[1]);

    return;
}  // end of HAL_setupADCTriggerTimer() function

void HAL_setupCPUUsageTimer(HAL_Handle handle)
{
    HAL_Obj  *obj = (HAL_Obj *)handle;

    // use timer 2 for CPU usage diagnostics
    CPUTimer_setPreScaler(obj->timerHandle[2], 0);

    CPUTimer_setEmulationMode(obj->timerHandle[2],
                              CPUTIMER_EMULATIONMODE_RUNFREE);

    CPUTimer_setPeriod(obj->timerHandle[2], 0xFFFFFFFF);

    CPUTimer_startTimer(obj->timerHandle[2]);

    return;
}  // end of HAL_setupCPUUsageTimer() function

#if defined(DATALOGF4_EN) || defined(DATALOGF2_EN)
void HAL_setupDMA(void)
{
    // Initializes the DMA controller to a known state
    DMA_initController();

    // Sets DMA emulation mode, Continue DMA operation
    DMA_setEmulationMode(DMA_EMULATION_FREE_RUN);

    return;
}    //end of HAL_setupDMA() function

void HAL_setupDMAforDLOG(HAL_Handle handle, const uint16_t dmaChNum,
                          const void *destAddr, const void *srcAddr)
{
    HAL_Obj *obj = (HAL_Obj *)handle;
    DMA_configAddresses(obj->dmaChHandle[dmaChNum], destAddr, srcAddr);

    // configure DMA Channel
    DMA_configBurst(obj->dmaChHandle[dmaChNum], DLOG_BURST_SIZE, 2, 2);
    DMA_configTransfer(obj->dmaChHandle[dmaChNum], DLOG_TRANSFER_SIZE, 1, 1);
    DMA_configWrap(obj->dmaChHandle[dmaChNum], 0xFFFF, 0, 0xFFFF, 0);

    DMA_configMode(obj->dmaChHandle[dmaChNum], DMA_TRIGGER_SOFTWARE,
                   DMA_CFG_ONESHOT_ENABLE | DMA_CFG_CONTINUOUS_ENABLE |
                   DMA_CFG_SIZE_32BIT);

    DMA_setInterruptMode(obj->dmaChHandle[dmaChNum], DMA_INT_AT_END);
    DMA_enableTrigger(obj->dmaChHandle[dmaChNum]);
    DMA_disableInterrupt(obj->dmaChHandle[dmaChNum]);

    return;
}    //end of HAL_setupDMAforDLOG() function
#endif  // DATALOGF4_EN || DATALOGF2_EN

void HAL_clearDataRAM(void *pMemory, uint16_t lengthMemory)
{
    uint16_t *pMemoryStart;
    uint16_t loopCount, loopLength;

    pMemoryStart = pMemory;
    loopLength = lengthMemory;

    for(loopCount = 0; loopCount < loopLength; loopCount++)
    {
        *(pMemoryStart + loopCount) = 0x0000;
    }
}   //end of HAL_clearDataRAM() function

void HAL_setMtrCMPSSDACValue(HAL_MTR_Handle handle,
                             const uint16_t dacValH, const uint16_t dacValL)
{
    HAL_MTR_Obj *obj = (HAL_MTR_Obj *)handle;

#if defined(MOTOR1_ISBLDC) || defined(MOTOR1_DCLINKSS)
#if defined(DRV8329AEVM_REVA)
    CMPSS_setDACValueHigh(obj->cmpssHandle[0], dacValH);
    // DRV8329AEVM_REVA
#else
#error This board doesn't support single shunt
#endif  // BSXL8323RS_REVA || BSXL8323RH_REVB || DRV8329AEVM_REVA
#else   // !(MOTOR1_ISBLDC || MOTOR1_DCLINKSS)
#if defined(BSXL8323RS_REVA) || defined(BSXL8323RH_REVB) || \
    defined(BSXL8353RS_REVA)

    CMPSS_setDACValueHigh(obj->cmpssHandle[0], dacValH);

    CMPSS_setDACValueHigh(obj->cmpssHandle[1], dacValH);
    CMPSS_setDACValueLow(obj->cmpssHandle[1], dacValL);

    CMPSS_setDACValueLow(obj->cmpssHandle[2], dacValL);
    // BSXL8323RS_REVA | BSXL8323RH_REVB | BSXL8353RS_REVA
#elif defined(HVMTRPFC_REV1P1) || defined(BSXL8316RT_REVA) || \
      defined(BSXL3PHGAN_REVA)
    CMPSS_setDACValueHigh(obj->cmpssHandle[0], dacValH);
    CMPSS_setDACValueLow(obj->cmpssHandle[0], dacValL);

    CMPSS_setDACValueHigh(obj->cmpssHandle[1], dacValH);
    CMPSS_setDACValueLow(obj->cmpssHandle[1], dacValL);

    CMPSS_setDACValueHigh(obj->cmpssHandle[2], dacValH);
    CMPSS_setDACValueLow(obj->cmpssHandle[2], dacValL);
    // HVMTRPFC_REV1P1 | BSXL8316RT_REVA | BSXL3PHGAN_REVA
#else
#error Not Select a Right Board
#endif  // 3-Shunt Hardware Board
#endif  // !(MOTOR1_ISBLDC || MOTOR1_DCLINKSS)

    return;
}   // end of HAL_setMtrCMPSSDACValue() function

void HAL_setTriggerPrams(HAL_PWMData_t *pPWMData, const float32_t systemFreq_MHz,
                   const float32_t deadband_us, const float32_t noiseWindow_us,
                   const float32_t adcSample_us)
{
    uint16_t deadband =  (uint16_t)(deadband_us * systemFreq_MHz);
    uint16_t noiseWindow =  (uint16_t)(noiseWindow_us * systemFreq_MHz);
    uint16_t adcSample =  (uint16_t)(adcSample_us * systemFreq_MHz);

    pPWMData->deadband = deadband;
    pPWMData->noiseWindow =noiseWindow;//syh
    pPWMData->adcSample = adcSample;

    pPWMData->minCMPValue = deadband + noiseWindow + adcSample;

    return;
}   // end of HAL_setTriggerPrams() function










bool HAL_MTR_setGateDriver(HAL_MTR_Handle handle)
{
    bool driverStatus = false;

    SysCtl_delay(5000U);

    // Setup Gate Enable
#if defined(DRV8329AEVM_REVA)
    // turn on the DRV8329A if present
    HAL_enableDRV(handle);

    // DRV8329AEVM_REVA
#elif defined(BSXL8323RS_REVA)
    // enable DRV8323RS
    HAL_setupGate(handle);
    SysCtl_delay(1000U);

    // turn on the DRV8323RS
    HAL_enableDRV(handle);
    SysCtl_delay(1000U);

    // initialize the DRV8323RS interface
    HAL_setupDRVSPI(handle, &drvicVars_M1);

    SysCtl_delay(1000U);

    drvicVars_M1.ctrlReg02.bit.OTW_REP = true;
    drvicVars_M1.ctrlReg02.bit.PWM_MODE = DRV8323_PWMMODE_6;

    drvicVars_M1.ctrlReg05.bit.VDS_LVL = DRV8323_VDS_LEVEL_1P700_V;
    drvicVars_M1.ctrlReg05.bit.OCP_MODE = DRV8323_AUTOMATIC_RETRY;
    drvicVars_M1.ctrlReg05.bit.DEAD_TIME = DRV8323_DEADTIME_100_NS;

    drvicVars_M1.ctrlReg06.bit.CSA_GAIN = DRV8323_Gain_10VpV;         // ADC_FULL_SCALE_CURRENT = 47.14285714A
//    drvicVars_M1.ctrlReg06.bit.CSA_GAIN = DRV8323_Gain_20VpV;       // ADC_FULL_SCALE_CURRENT = 23.57142857A
    drvicVars_M1.ctrlReg06.bit.LS_REF = false;
    drvicVars_M1.ctrlReg06.bit.VREF_DIV = true;
    drvicVars_M1.ctrlReg06.bit.CSA_FET = false;

    drvicVars_M1.writeCmd = 1;
    HAL_writeDRVData(handle, &drvicVars_M1);
    SysCtl_delay(1000U);

    drvicVars_M1.writeCmd = 1;
    HAL_writeDRVData(handle, &drvicVars_M1);
    SysCtl_delay(1000U);

    // BSXL8323RS_REVA
#elif defined(BSXL8323RH_REVB)
    // turn on the DRV8323RH if present
    HAL_enableDRV(handle);
    SysCtl_delay(1000U);

    // BSXL8323RH_REVB
#elif defined(BSXL8353RS_REVA)
    // enable DRV8353RS
    HAL_setupGate(handle);
    SysCtl_delay(1000U);

    // turn on the DRV8353RS
    HAL_enableDRV(handle);
    SysCtl_delay(1000U);

    // initialize the DRV8353RS interface
    HAL_setupDRVSPI(handle, &drvicVars_M1);
    SysCtl_delay(1000U);

    drvicVars_M1.ctrlReg03.bit.IDRIVEP_HS = DRV8353_ISOUR_HS_0P820_A;
    drvicVars_M1.ctrlReg03.bit.IDRIVEN_HS = DRV8353_ISINK_HS_1P640_A;

    drvicVars_M1.ctrlReg04.bit.IDRIVEP_LS = DRV8353_ISOUR_LS_0P820_A;
    drvicVars_M1.ctrlReg04.bit.IDRIVEN_LS = DRV8353_ISINK_LS_1P640_A;

    drvicVars_M1.ctrlReg05.bit.VDS_LVL = DRV8353_VDS_LEVEL_1P500_V;
    drvicVars_M1.ctrlReg05.bit.OCP_MODE = DRV8353_LATCHED_SHUTDOWN;
    drvicVars_M1.ctrlReg05.bit.DEAD_TIME = DRV8353_DEADTIME_100_NS;

    drvicVars_M1.ctrlReg06.bit.CSA_GAIN = DRV8353_Gain_10VpV;
    drvicVars_M1.ctrlReg06.bit.LS_REF = false;
    drvicVars_M1.ctrlReg06.bit.VREF_DIV = true;
    drvicVars_M1.ctrlReg06.bit.CSA_FET = false;

    // write DRV8353RS control registers
    drvicVars_M1.writeCmd = 1;
    HAL_writeDRVData(handle, &drvicVars_M1);
    SysCtl_delay(1000U);

    // write DRV8353RS control registers again
    drvicVars_M1.writeCmd = 1;
    HAL_writeDRVData(handle, &drvicVars_M1);
    SysCtl_delay(1000U);

    // BSXL8353RS_REVA
#elif defined(BSXL8316RT_REVA)
    // enable DRV8316RT
    HAL_setupGate(handle);
    SysCtl_delay(1000U);

    // turn on the DRV8316RT
    HAL_enableDRV(handle);
    SysCtl_delay(1000U);

    // initialize the DRV8316RT interface
    HAL_setupDRVSPI(handle, &drvicVars_M1); // new board
    SysCtl_delay(1000U);
    drvicVars_M1.ctrlReg01.bit.REG_LOCK = DRV8316_REG_LOCK_3;
    drvicVars_M1.ctrlReg02.bit.PWM_MODE = DRV8316_PWMMODE_6_N;
    drvicVars_M1.ctrlReg02.bit.SLEW =DRV8316_SLEW_25V;// DRV8316_SLEW_50V;

    // Don't change the CSA_GAIN! If changes this setting value,
    // USER_M1_ADC_FULL_SCALE_CURRENT_A must be changed in user_mtr1.h accordingly
    drvicVars_M1.ctrlReg05.bit.CSA_GAIN =DRV8316_CSA_GAIN_0p30VpA;// syh newboard 

    drvicVars_M1.ctrlReg06.bit.BUCK_DIS = false;//true; buck off /syh
    drvicVars_M1.ctrlReg06.bit.BUCK_SEL = DRV8316_BUCK_SEL_3p3V;
    drvicVars_M1.ctrlReg06.bit.BUCK_CL = DRV8316_BUCK_CL_150mA;
    drvicVars_M1.ctrlReg03.bit.PWM_100_DUTY_SEL=DRV8316_PWM_100_DUTY_SEL_40KHz;

    

    // write DRV8316RT control registers
    drvicVars_M1.writeCmd = 1;
    HAL_writeDRVData(handle, &drvicVars_M1);
    SysCtl_delay(1000U);

    // write DRV8316RT control registers again
   /* drvicVars_M1.writeCmd = 1;
    HAL_writeDRVData(handle, &drvicVars_M1);
    SysCtl_delay(1000U);*/
    drvicVars_M1.readCmd = 1;
    HAL_readDRVData(handle, &drvicVars_M1);
    SysCtl_delay(1000U);

    // BSXL8316RT_REVA
#elif defined(BSXL3PHGAN_REVA)
    // turn on the 3PhGaN if present
    HAL_enableDRV(handle);
    SysCtl_delay(1000U);

    // BSXL3PHGAN_REVA
#elif defined(HVMTRPFC_REV1P1)
    // turn on the HvKit if present
    HAL_enableDRV(handle);
    SysCtl_delay(1000U);

    //HVMTRPFC_REV1P1
#else
#error Not select a right supporting kit!
#endif  // Setup Gate Enable

    return(driverStatus);
}


// end of file
