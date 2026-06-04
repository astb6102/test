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


//! \file   /solutions/universal_motorcontrol_lab/common/source/communication.c
//!
//! \brief  This project is used to implement motor control with FAST, eSMO
//!         Encoder, and Hall sensors based sensored/sensorless-FOC.
//!         Supports multiple TI EVM boards
//!

//
// include the related header files
//
#include "sys_main.h"

#include "communication.h"
#include "motor1_drive.h"
#include "motor_common.h"

// EEPROM 기록 함수 외부 선언 (sys_main.c 에 정의됨)
extern void EEPROM_writeFloatByIndex(uint8_t floatIndex, float value);




//
// Defines
//



#if defined(CMD_CAN_EN)

#pragma CODE_SECTION(canaISR, ".TI.ramfunc");

// **************************************************************************
// the globals

volatile CANCOM_Obj canComVars;
int CAN_USER_ID;//syh
float32_t temp_calculation_float1;
int16_t temp_calculation_int1;

float32_t temp_calculation_float2;
int16_t temp_calculation_int2;

float32_t temp_calculation_float3;
int16_t temp_calculation_int3;
 uint32_t temp_can_status;
//bool bitArray1[8]={};
//bool bitArray2[8]={};
float32_t preset1;
float32_t preset2;
    float32_t preset3;
bool flagEnablePosCtrl;

//extern volatile MOTORVarst motorVarsM1; 


// **************************************************************************
// the functions
void HAL_setupCAN(HAL_Handle halHandle)
{
    HAL_Obj *obj = (HAL_Obj *)halHandle;

    // Initialize the CAN controller
    CAN_initModule(obj->canHandle);

    // Set up the CAN bus bit rate to 400kHz
    // Refer to the Driver Library User Guide for information on how to set
    // tighter timing control. Additionally, consult the device data sheet
    // for more information about the CAN module clocking.
    CAN_setBitRate(obj->canHandle, DEVICE_SYSCLK_FREQ, 500000, 16);

    // Initialize the transmit message object used for sending CAN messages.
    // Message Object Parameters:
    //      Message Object ID Number: 1
    //      Message Identifier: 0x1
    //      Message Frame: Standard
    //      Message Type: Transmit
    //      Message ID Mask: 0x0
    //      Message Object Flags: Transmit Interrupt
    //      Message Data Length: 8 Bytes
    CAN_setupMessageObject(obj->canHandle, TX_MSG_OBJ_ID, 0x1, CAN_MSG_FRAME_STD,
                           CAN_MSG_OBJ_TYPE_TX, 0, CAN_MSG_OBJ_TX_INT_ENABLE,
                           MSG_DATA_LENGTH);

    // Initialize the receive message object used for receiving CAN messages.
    // Message Object Parameters:
    //      Message Object ID Number: 2
    //      Message Identifier: 0x1
    //      Message Frame: Standard
    //      Message Type: Receive
    //      Message ID Mask: 0x0
    //      Message Object Flags: Receive Interrupt
    //      Message Data Length: 8 Bytes
    CAN_setupMessageObject(obj->canHandle, RX_MSG_OBJ_ID, 0x1, CAN_MSG_FRAME_STD,
                           CAN_MSG_OBJ_TYPE_RX, 0, CAN_MSG_OBJ_RX_INT_ENABLE,
                           MSG_DATA_LENGTH);
    CAN_setupMessageObject(obj->canHandle, 3, 0x2, CAN_MSG_FRAME_STD,
                           CAN_MSG_OBJ_TYPE_TX, 0, CAN_MSG_OBJ_TX_INT_ENABLE,
                           MSG_DATA_LENGTH);
    // Initialize the receive message object 4 for SETUP parameters (ID: 0x10)
    CAN_setupMessageObject(obj->canHandle, RX_SETUP_OBJ_ID, 0x10,
                           CAN_MSG_FRAME_STD, CAN_MSG_OBJ_TYPE_RX, 0,
                           CAN_MSG_OBJ_RX_INT_ENABLE, MSG_DATA_LENGTH);

    // [추가] Initialize the transmit message object 5 for SETUP RESPONSE (ID: 0x11)
    CAN_setupMessageObject(obj->canHandle, TX_SETUP_OBJ_ID, 0x11,
                           CAN_MSG_FRAME_STD, CAN_MSG_OBJ_TYPE_TX, 0,
                           CAN_MSG_OBJ_NO_FLAGS, MSG_DATA_LENGTH);

    // Start CAN module operations
    CAN_startModule(obj->canHandle);

    // Enable CAN test mode with external loopback
//     CAN_enableTestMode(halHandle->canHandle, CAN_TEST_EXL);    // Only for debug

    // Enable interrupts on the CAN peripheral.
    CAN_enableInterrupt(obj->canHandle, CAN_INT_IE0 | CAN_INT_ERROR |
                        CAN_INT_STATUS);

    CAN_enableGlobalInterrupt(obj->canHandle, CAN_GLOBAL_INT_CANINT0);

    // enable the PIE interrupts associated with the CAN interrupts
    Interrupt_enable(COM_INT_CAN);

    // enable the cpu interrupt for CAN interrupts
    Interrupt_enableInCPU(INTERRUPT_CPU_INT9);

    return;
}  // end of DRV_setupSci() function

//! \brief      Initializes CAN
//! \param[in]  N/A
void initCANCOM(HAL_Handle handle)
{
    GPIO_setPinConfig(COM_CANRX_GPIO_PIN_CONFIG);
    GPIO_setDirectionMode(COM_CANRX_GPIO, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(COM_CANRX_GPIO, GPIO_QUAL_ASYNC);

    GPIO_setPinConfig(COM_CANTX_GPIO_PIN_CONFIG);
    GPIO_setDirectionMode(COM_CANTX_GPIO, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(COM_CANTX_GPIO, GPIO_QUAL_ASYNC);

    // setup the CAN
    HAL_setupCAN(handle);

    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
    // This registers the interrupt handler in PIE vector table.
    Interrupt_register(COM_INT_CAN, &canaISR);

    canComVars.speedConv_sf = 0.1f;          // uint16->float(unit=0.1Hz)
    canComVars.speedInv_sf = 10.0f;          // float->uint16(unit=0.1Hz)
    canComVars.currentConv_sf = 0.01f;       // uint16->float(unit=0.01A)
    canComVars.currentInv_sf = 100.0f;       // float->uint16(unit=0.01A)

    canComVars.txMsgCount = 0;                // for debug
    canComVars.rxMsgCount = 0;                // for debug

    canComVars.waitTimeCnt = 1000;            // 1s/1000ms
    canComVars.waitTimeDelay = 2;             // 2ms

    canComVars.flagTxDone = true;             // To enable CAN
    canComVars.flagRxDone = false;

    canComVars.speedRef_Hz = 0.0f;            // 0Hz
    canComVars.speedSet_Hz = 40.0f;           // 40Hz

    canComVars.flagCmdTxRun = false;
    canComVars.flagCmdRxRun = false;

    return;
} // end of HAL_initCANInt() function


void updateCANCmdFreq(MOTOR_Handle handle)
{
    MOTOR_Vars_t *objMtr = (MOTOR_Vars_t *)handle;
    int16_t canData = 0;
    int i;
    float temp_hz;

    if(canComVars.flagRxDone == true)
    {
       /*
        canComVars.flagCmdRxRun = (bool)(canComVars.rxMsgData[0]);
        canComVars.motorStateRx = (MOTOR_Status_e)(canComVars.rxMsgData[1]);

        canComVars.speedRef_Hz = ((float32_t)((canComVars.rxMsgData[2]<<8) +
                canComVars.rxMsgData[3])) * canComVars.speedConv_sf;

        canComVars.speedRx_Hz = ((float32_t)((canComVars.rxMsgData[4]<<8) +
                canComVars.rxMsgData[5])) * canComVars.speedConv_sf;

        canComVars.IqRx_A = ((float32_t)((canComVars.rxMsgData[6]<<8) +
                canComVars.rxMsgData[7])) * canComVars.currentConv_sf;
        */
       for (i = 0; i < 8; i++)
    {
        motorVars_M1.bitArray1[i] = (canComVars.rxMsgData[0]>> i) & 0x01;//can cmd/position cont/run/error clr/preset 1,2,3(run set), preest input
        motorVars_M1.bitArray2[i] = (canComVars.rxMsgData[1]>> i) & 0x01;
    }
    
     /*   for (i = 0; i < 8; i++)
    {
        bitArray2[i] = (canComVars.rxMsgData[1]>> i) & 0x01;//can cmd/position cont/run/error clr/preset 1,2,3(run set), preest input
    } */



    if(motorVars_M1.bitArray1[0]==1)
    {
    flagEnablePosCtrl=motorVars_M1.bitArray1[1];
    motorVars_M1.flagEnableRunAndIdentify=motorVars_M1.bitArray1[2];

     if(motorVars_M1.bitArray1[3]==1)
        motorVars_M1.faultMtrNow.all=0;
    
    if(motorVars_M1.bitArray1[4]==1)
     {
       motorVars_M1.target_pos= preset1;
     }
     if(motorVars_M1.bitArray1[5]==1)
     {
        motorVars_M1.target_pos= preset2;
     }
     if(motorVars_M1.bitArray1[6]==1)
     {
        motorVars_M1.target_pos= preset3;
     }
    
     if(motorVars_M1.bitArray1[7]==1)
     {
     preset1=((float32_t)((canComVars.rxMsgData[2]<<8) +
                canComVars.rxMsgData[3])) * canComVars.speedConv_sf;
    preset2=((float32_t)((canComVars.rxMsgData[4]<<8) +
                canComVars.rxMsgData[5])) * canComVars.speedConv_sf;
     preset3=((float32_t)((canComVars.rxMsgData[6]<<8) +
                canComVars.rxMsgData[7])) * canComVars.speedConv_sf;           

     }
     else {
        if(flagEnablePosCtrl==1)
           {
            if(motorVars_M1.bitArray2[1]!=1)
            {
            motorVars_M1.target_pos=((float32_t)((canComVars.rxMsgData[2]<<8) +
            canComVars.rxMsgData[3])) * canComVars.speedConv_sf;
            }
           }
         else {
           if(motorVars_M1.bitArray2[0]==1)
           {
           motorVars_M1.speedRef_Hz=((float32_t)((canComVars.rxMsgData[2]<<8) +
                canComVars.rxMsgData[3])) * canComVars.speedConv_sf;
           }
           else {
           motorVars_M1.speedRef_Hz=((float32_t)((canComVars.rxMsgData[2]<<8) +
                canComVars.rxMsgData[3])) * canComVars.speedConv_sf*-1;
           }
         }
         if(motorVars_M1.bitArray2[2]==1)
         motorVars_M1.Current_limit= ((float32_t)((canComVars.rxMsgData[4]<<8) +
                canComVars.rxMsgData[5])) * canComVars.speedConv_sf;
                else
                 motorVars_M1.Torque_limit= ((float32_t)((canComVars.rxMsgData[4]<<8) +
                canComVars.rxMsgData[5])) * canComVars.speedConv_sf;;
     //spare 6,7 -syh
     }
     

    }

       canComVars.flagTxDone = true;//syh
        canComVars.flagRxDone = false;
       // canComVars.flagTxDone = true;
        
    }

    if((canComVars.flagTxDone == true) && (canComVars.waitTimeCnt == 0))
    {
       // if(CAN_USER_ID==1) syh
       // {
        canComVars.txMsgData[0] =(int16_t)motorVars_M1.faultMtrNow.all; //(int16_t)(canComVars.flagCmdTxRun); ///SYH INT 1
       
        canComVars.txMsgData[1] =(int16_t)fabs(objMtr->speed_Hz);//SYH INT 2
        
    
    
        //1
    
        temp_calculation_float1=motorVars_M1.txdata[0]*canComVars.speedInv_sf;//*10.0f;
        temp_calculation_int1=(int16_t)temp_calculation_float1;
   
        canData = (int16_t)(temp_calculation_int1);//(canComVars.speedSet_Hz * canComVars.speedInv_sf);//SYH FLOAT 1
        canComVars.txMsgData[2] = (canData>>8) & 0x00FF;
        canComVars.txMsgData[3] = canData & 0x00FF;

        
        //2
        temp_calculation_float2=motorVars_M1.txdata[1]*canComVars.speedInv_sf;//*10.0f;
        temp_calculation_int2=(int16_t)temp_calculation_float2;

        canData = (int16_t)(temp_calculation_int2);//(objMtr->speedAbs_Hz * canComVars.speedInv_sf);//SYH FLOAT 2
        canComVars.txMsgData[4] = (canData>>8) & 0x00FF;
        canComVars.txMsgData[5] = canData & 0x00FF;
        
        
        //3
         temp_calculation_float3=motorVars_M1.txdata[2]*canComVars.speedInv_sf;//*10.0f;
        temp_calculation_int3=(int16_t)temp_calculation_float3;

        canData = (int16_t)(temp_calculation_int3);//(objMtr->Idq_in_A.value[1] * canComVars.currentInv_sf);//SYH FLOAT 3

        canComVars.txMsgData[6] = (canData>>8) & 0x00FF;
        canComVars.txMsgData[7] = canData & 0x00FF;

        CAN_sendMessage(halHandle->canHandle, 0x01, MSG_DATA_LENGTH,(int16_t *)(&canComVars.txMsgData[0]));//uint -> int
       // canComVars.waitTimeCnt = canComVars.waitTimeDelay;
        canComVars.txMsgData[0] =(int16_t)motorVars_M1.faultMtrNow.all; //(int16_t)(canComVars.flagCmdTxRun); ///SYH INT 1
        canComVars.txMsgData[1] =(int16_t)fabs(objMtr->speed_Hz);//SYH INT 2
        

        temp_calculation_float1=motorVars_M1.txdata[3]*canComVars.speedInv_sf;//*10.0f;
        temp_calculation_int1=(int16_t)temp_calculation_float1;
   
        canData = (int16_t)(temp_calculation_int1);//(canComVars.speedSet_Hz * canComVars.speedInv_sf);//SYH FLOAT 1
        canComVars.txMsgData[2] = (canData>>8) & 0x00FF;
        canComVars.txMsgData[3] = canData & 0x00FF;

        
        //2
        temp_calculation_float2=motorVars_M1.txdata[4]*canComVars.speedInv_sf;//*10.0f;
        temp_calculation_int2=(int16_t)temp_calculation_float2;

        canData = (int16_t)(temp_calculation_int2);//(objMtr->speedAbs_Hz * canComVars.speedInv_sf);//SYH FLOAT 2
        canComVars.txMsgData[4] = (canData>>8) & 0x00FF;
        canComVars.txMsgData[5] = canData & 0x00FF;
        
        
        //3
         temp_calculation_float3=motorVars_M1.txdata[5]*canComVars.speedInv_sf;//*10.0f;
        temp_calculation_int3=(int16_t)temp_calculation_float3;

        canData = (int16_t)(temp_calculation_int3);//(objMtr->Idq_in_A.value[1] * canComVars.currentInv_sf);//SYH FLOAT 3

        canComVars.txMsgData[6] = (canData>>8) & 0x00FF;
        canComVars.txMsgData[7] = canData & 0x00FF;

        CAN_sendMessage(halHandle->canHandle, 0x03 , MSG_DATA_LENGTH,(int16_t *)(&canComVars.txMsgData[0]));//uint -> int
        
        canComVars.waitTimeCnt = canComVars.waitTimeDelay;
        canComVars.flagTxDone = false;
       // }
    }

    if(canComVars.waitTimeCnt > 0)
    {
        canComVars.waitTimeCnt--;
    }

    return;
}
// 셋업 데이터 처리 및 응답 함수
void updateCANSetupParams(void)
{
    if(canComVars.flagSetupRxDone == true)
    {
        // 워드 0: 하위 바이트는 Command, 상위 바이트는 Index
        uint16_t cmd   = canComVars.rxSetupData[0] & 0x00FF; 
        uint16_t index = (canComVars.rxSetupData[0] >> 8) & 0x00FF; 
        
        union {
            uint32_t raw;
            float fVal;
        } dataCvt;

        // ----------------------------------------------------
        // Command 1, 2: 파라미터 쓰기 (Write) 모드
        // ----------------------------------------------------
        if(cmd == 1 || cmd == 2) 
        {
            dataCvt.raw = ((uint32_t)canComVars.rxSetupData[3] << 16) | (uint16_t)canComVars.rxSetupData[2];
            float receivedValue = dataCvt.fVal;

            // RAM 변수 업데이트
            switch(index)
            {
                case 1: motorVars_M1.pos_Kp = receivedValue; break;
                case 2: motorVars_M1.pos_Ki = receivedValue; break;
                case 3: preset1 = receivedValue; break;
                case 4: preset2 = receivedValue; break;
                case 5: preset3 = receivedValue; break;
                default: break;
            }

            // Command가 1일 경우에만 EEPROM에 영구 저장
            if(cmd == 1)
            {
                EEPROM_writeFloatByIndex(index, receivedValue);
            }
        }
        // ----------------------------------------------------
        // Command 3: 파라미터 읽기 (Read) 모드
        // ----------------------------------------------------
        else if(cmd == 3)
        {
            float readValue = 0.0f;
            
            // 현재 시스템(RAM)에 적용되어 있는 값 로드
            switch(index)
            {
                case 1: readValue = motorVars_M1.pos_Kp; break;
                case 2: readValue = motorVars_M1.pos_Ki; break;
                case 3: readValue = preset1; break;
                case 4: readValue = preset2; break;
                case 5: readValue = preset3; break;
                default: break;
            }
            
            dataCvt.fVal = readValue;
            
            // 응답 데이터 패키징 (수신된 명령어와 인덱스를 그대로 에코)
            canComVars.txSetupData[0] = (index << 8) | cmd; 
            canComVars.txSetupData[1] = 0x0000;             // Reserved
            canComVars.txSetupData[2] = (uint16_t)(dataCvt.raw & 0xFFFF);        // LSW
            canComVars.txSetupData[3] = (uint16_t)((dataCvt.raw >> 16) & 0xFFFF);// MSW
            
            // CAN 하드웨어 모듈(CANA_BASE)을 통해 ID 0x11로 즉시 송신
            CAN_sendMessage(CANA_BASE, TX_SETUP_OBJ_ID, MSG_DATA_LENGTH, (uint16_t *)canComVars.txSetupData);
        }
        
        canComVars.flagSetupRxDone = false;
    }
}

void CAN_B_DATA_READ(MOTOR_Handle handle)
{
    MOTOR_Vars_t *objMtr = (MOTOR_Vars_t *)handle;
    int16_t canData = 0;
    int i;
    float temp_hz;
    
    if(rx[3]!=0 )
    {
         for (i = 0; i < 8; i++)
        {
            motorVars_M1.bitArray1[i] = ( ((rx2[3] >> 0) & 0xFF)>> i) & 0x01;//can cmd/position cont/run/error clr/preset 1,2,3(run set), preest input
            motorVars_M1.bitArray2[i] = ( ((rx2[3] >> 8) & 0xFF)>> i) & 0x01;
        }
    
    if(motorVars_M1.bitArray1[0]==1)
    {
         flagEnablePosCtrl=motorVars_M1.bitArray1[1];
         motorVars_M1.flagEnableRunAndIdentify=motorVars_M1.bitArray1[2];

        if(motorVars_M1.bitArray1[3]==1)
         motorVars_M1.faultMtrNow.all=0;
    
        if(motorVars_M1.bitArray1[4]==1)
        {
         motorVars_M1.target_pos= preset1;
        }
         if(motorVars_M1.bitArray1[5]==1)
        {
            motorVars_M1.target_pos= preset2;
         }
        if(motorVars_M1.bitArray1[6]==1)
        {
          motorVars_M1.target_pos= preset3;
         }
    
        if(motorVars_M1.bitArray1[7]==1)
        {
            preset1=((float32_t)((((rx2[2] >> 0) & 0xFF)<<8) +
                ((rx2[2] >> 8) & 0xFF))) * canComVars.speedConv_sf;
            preset2=((float32_t)((((rx2[5] >> 0) & 0xFF)<<8) +
               ((rx2[5] >> 8) & 0xFF))) * canComVars.speedConv_sf;
            preset3=((float32_t)((((rx2[4] >> 0) & 0xFF)<<8) +
                ((rx2[4] >> 8) & 0xFF))) * canComVars.speedConv_sf;           

        }
         else 
         {
             if(flagEnablePosCtrl==1)
             {
                 if(motorVars_M1.bitArray2[1]!=1)
                 {
                    motorVars_M1.target_pos=((float32_t)((((rx2[2] >> 0) & 0xFF)<<8) +
                     ((rx2[2] >> 8) & 0xFF))) * canComVars.speedConv_sf;
                 }
             }
            else
            {
            if(motorVars_M1.bitArray2[0]==1)
                {
                    motorVars_M1.speedRef_Hz=((float32_t)((((rx2[2] >> 0) & 0xFF)<<8) +
                    ((rx2[2] >> 8) & 0xFF))) * canComVars.speedConv_sf;
                 }
           else 
                {
                    motorVars_M1.speedRef_Hz=((float32_t)((((rx2[2] >> 0) & 0xFF)<<8) +
                    ((rx2[2] >> 8) & 0xFF))) * canComVars.speedConv_sf*-1;
                }
         }

         if(motorVars_M1.bitArray2[2]==1)
                motorVars_M1.Current_limit= ((float32_t)((((rx2[5] >> 0) & 0xFF)<<8) +
                ((rx2[5] >> 8) & 0xFF))) * canComVars.speedConv_sf;
        else
                 motorVars_M1.Torque_limit= ((float32_t)((((rx2[5] >> 0) & 0xFF)<<8) +
                ((rx2[5] >> 8) & 0xFF))) * canComVars.speedConv_sf;;
    
     }
     
    }
    }
       

        B_CAN_TX[0] =(int16_t)motorVars_M1.faultMtrNow.all; //(int16_t)(canComVars.flagCmdTxRun); ///SYH INT 1
        B_CAN_TX[1] =(int16_t)fabs(objMtr->speed_Hz);//SYH INT 2
    //1
    //    temp_calculation_float1=motorVars_M1.txdata1*canComVars.speedInv_sf;//*10.0f;
    //    temp_calculation_int1=(int16_t)temp_calculation_float1+1000;
    //    canData = (int16_t)(temp_calculation_int1);//(canComVars.speedSet_Hz * canComVars.speedInv_sf);//SYH FLOAT 1
      
        B_CAN_TX[2] = (canData>>8) & 0x00FF;
        B_CAN_TX[3] = canData & 0x00FF;

        tx_data[0] =   ((uint32_t)B_CAN_TX[3] << 24) |
                       ((uint32_t)B_CAN_TX[2] << 16) |
                       ((uint32_t)B_CAN_TX[1] << 8)  |
                       ((uint32_t)B_CAN_TX[0]);
    //2
    //    temp_calculation_float2=motorVars_M1.txdata2*canComVars.speedInv_sf;//*10.0f;
    //    temp_calculation_int2=(int16_t)temp_calculation_float2;
    //    canData = (int16_t)(temp_calculation_int2);//(objMtr->speedAbs_Hz * canComVars.speedInv_sf);//SYH FLOAT 2
       
        B_CAN_TX[4] = (canData>>8) & 0x00FF;
        B_CAN_TX[5] = canData & 0x00FF;
        
  //3
  //       temp_calculation_float3=motorVars_M1.txdata3*canComVars.speedInv_sf;//*10.0f;
  //      temp_calculation_int3=(int16_t)temp_calculation_float3;
  //      canData = (int16_t)(temp_calculation_int3);//(objMtr->Idq_in_A.value[1] * canComVars.currentInv_sf);//SYH FLOAT 3

        B_CAN_TX[6] = (canData>>8) & 0x00FF;
        B_CAN_TX[7] = canData & 0x00FF;

        tx_data[1] =   ((uint32_t)B_CAN_TX[7] << 24) |
                       ((uint32_t)B_CAN_TX[6] << 16) |
                       ((uint32_t)B_CAN_TX[5] << 8)  |
                       ((uint32_t)B_CAN_TX[4]);
    
    return;
}

__interrupt void canaISR(void) // can isr syh
{
    uint32_t status;

    temp_can_status=status;

    // Read the CAN interrupt status to find the cause of the interrupt
    status = CAN_getInterruptCause(halHandle->canHandle);



    // If the cause is a controller status interrupt, then get the status
    if(status == CAN_INT_INT0ID_STATUS)
    {
        //
        // Read the controller status.  This will return a field of status
        // error bits that can indicate various errors.  Error processing
        // is not done in this example for simplicity.  Refer to the
        // API documentation for details about the error status bits.
        // The act of reading this status will clear the interrupt.
        status = CAN_getStatus(halHandle->canHandle);
        CAN_USER_ID=status;

        // Check to see if an error occurred.
        if(((status  & ~(CAN_STATUS_TXOK | CAN_STATUS_RXOK)) != 7) &&
           ((status  & ~(CAN_STATUS_TXOK | CAN_STATUS_RXOK)) != 0))
        {
            // Set a flag to indicate some errors may have occurred.
            canComVars.errorFlag = 1;
        }

    }
    // Check if the cause is the transmit message object 1
    else if(status == TX_MSG_OBJ_ID|| (status == 3))
    {
        //
        // Getting to this point means that the TX interrupt occurred on
        // message object 1, and the message TX is complete.  Clear the
        // message object interrupt.
        //
        CAN_clearInterruptStatus(halHandle->canHandle, status);

        // Increment a counter to keep track of how many messages have been
        // sent.  In a real application this could be used to set flags to
        // indicate when a message is sent.
        canComVars.txMsgCount++;

        // Since the message was sent, clear any error flags.
        canComVars.errorFlag = 0;
    }

    // Check if the cause is the receive message object 2
    else if(status == RX_MSG_OBJ_ID) /// syh _ can id sorting
    {
        //
        // Get the received message
        //
        CAN_readMessage(halHandle->canHandle, RX_MSG_OBJ_ID,
                        (uint16_t *)(&canComVars.rxMsgData[0]));

        // Getting to this point means that the RX interrupt occurred on
        // message object 2, and the message RX is complete.  Clear the
        // message object interrupt.
        CAN_clearInterruptStatus(halHandle->canHandle, RX_MSG_OBJ_ID);

        canComVars.rxMsgCount++;
        canComVars.flagRxDone = true;

        // Since the message was received, clear any error flags.
        canComVars. errorFlag = 0;
    }
    else if(status == RX_SETUP_OBJ_ID)
    {
        // 메일박스 4번에서 데이터 읽기
        CAN_readMessage(halHandle->canHandle, RX_SETUP_OBJ_ID,
                        (uint16_t *)(&canComVars.rxSetupData[0]));
        
        // 인터럽트 클리어 및 수신 완료 플래그 세팅
        CAN_clearInterruptStatus(halHandle->canHandle, RX_SETUP_OBJ_ID);
        canComVars.flagSetupRxDone = true;
        canComVars.errorFlag = 0;
    }
    // If something unexpected caused the interrupt, this would handle it.
    else
    {
        // Spurious interrupt handling can go here.
    }

    // Clear the global interrupt flag for the CAN interrupt line
    CAN_clearGlobalInterruptStatus(halHandle->canHandle, CAN_GLOBAL_INT_CANINT0);

    // Acknowledge this interrupt located in group 9
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);

    return;
}
#else
__interrupt void canaISR(void)
{
    // Acknowledge this interrupt located in group 9
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
}
#endif // CMD_CAN_EN
//
//-- end of this file ----------------------------------------------------------
//
