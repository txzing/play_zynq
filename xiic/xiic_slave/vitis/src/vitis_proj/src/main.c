/******************************************************************************
* Copyright (C) 2006 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
* @file xiic_slave_example.c
*
* This file consists of a Interrupt mode design example which uses the Xilinx
* IIC device and XIic driver to exercise the slave functionality of the IIC
* device.
*
* The XIic_SlaveSend() API is used to transmit the data and
* XIic_SlaveRecv() API is used to receive the data.
*
* The example is tested on ML300/ML310/ML403/ML501 Xilinx boards.
*
* The IIC devices that are present on the Xilinx boards donot support the Master
* functionality. This example has been tested with an off board external IIC
* Master device and the IIC device configured as a Slave.
*
* This code assumes that no Operating System is being used.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a mta  03/01/06 Created.
* 2.00a ktn  11/17/09 Updated to use the HAL APIs and replaced call to
*		      XIic_Initialize API with XIic_LookupConfig and
*		      XIic_CfgInitialize. Some of the macros have been
*		      renamed in the IIC driver and some renamed macros are
*		      used in this example.
* 3.4   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xiic.h"
#if defined (ARMR5) || (__aarch64__) || (__arm__)
#include "xscugic.h"
#else
#include "xintc.h"
#endif
#include "xil_exception.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */

#if defined (ARMR5) || (__aarch64__) || (__arm__)
	#define IIC_DEVICE_ID		XPAR_IIC_0_DEVICE_ID
	#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
	#define IIC_INT_VEC_ID		XPAR_FABRIC_IIC_0_VEC_ID
#else
	#define IIC_DEVICE_ID		XPAR_IIC_0_DEVICE_ID
	#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
	#define IIC_INTR_ID		XPAR_INTC_0_IIC_0_VEC_ID
#endif

/*
 * The following constant defines the address of the IIC device on the IIC bus.
 * Since the address is only 7 bits, this constant is the address divided by 2.
 */
#define SLAVE_ADDRESS       0x50    /* 0xE0 as an 8 bit number. */

#define RECEIVE_COUNT       30
#define SEND_COUNT          30


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int IicSlaveExample();
int SlaveWriteData(u16 ByteCount);
int SlaveReadData(u8 *BufferPtr, u16 ByteCount);
static int SetupInterruptSystem(XIic * IicInstPtr);
static void StatusHandler(XIic *InstancePtr, int Event);
static void SendHandler(XIic *InstancePtr);
static void ReceiveHandler(XIic *InstancePtr);

/************************** Variable Definitions *****************************/

XIic IicInstance;		/* The instance of the IIC device. */
XIicStats IicStatus;
#if defined (ARMR5) || (__aarch64__) || (__arm__)
XScuGic InterruptController;
#else
XIntc InterruptController;	/* The instance of the Interrupt Controller */
#endif

u8 WriteBuffer[SEND_COUNT];	/* Write buffer for writing a page. */
u8 WriteBuffer_1[SEND_COUNT];
u8 ReadBuffer[RECEIVE_COUNT];	/* Read buffer for reading a page. */

volatile u8 TransmitComplete;
volatile u8 ReceiveComplete;

volatile u8 SlaveRead;
volatile u8 SlaveWrite;
volatile u8 SlaveReadWrite;
volatile u8 SlaveReadBytes;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* Main function to call the IIC Slave example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
    int Status;

    /*
     * Run the IIC Slave example.
     */
    xil_printf("i2c slave example begin\n");
    Status = IicSlaveExample();
    if (Status != XST_SUCCESS) {
		xil_printf("IIC slave Example Failed\r\n");
        return XST_FAILURE;
    }

	xil_printf("Successfully ran IIC slave Example\r\n");
    return XST_SUCCESS;
}

void delay_SL(u32 delayCount) {
    do {
        __asm__("nop");
        delayCount--;
    } while (delayCount > 0);
}
/*****************************************************************************/
/**
* This function writes and reads the data as a slave. The IIC master on the bus
* initiates the transfers.
*
* @param    None.
*
* @return   XST_SUCCESS if successful else XST_FAILURE.
*
* @note     None.
*
******************************************************************************/
int IicSlaveExample()
{
    int Status;
    u8 Index;
    XIic_Config *ConfigPtr; /* Pointer to configuration data */


    /*
     * Initialize the IIC driver so that it is ready to use.
     */
    ConfigPtr = XIic_LookupConfig(IIC_DEVICE_ID);
    if (ConfigPtr == NULL) {
        return XST_FAILURE;
    }

    Status = XIic_CfgInitialize(&IicInstance, ConfigPtr,
                    ConfigPtr->BaseAddress);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    /*
     * Setup the Interrupt System.
     */
    Status = SetupInterruptSystem(&IicInstance);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    xil_printf("i2c slave example begin test1\r\n");
    /*
     * Include the Slave functions.
     */
    XIic_SlaveInclude();

    /*
     * Set the Transmit, Receive and Status Handlers.
     */
    XIic_SetStatusHandler(&IicInstance, &IicInstance,
                  (XIic_StatusHandler) StatusHandler);
    XIic_SetSendHandler(&IicInstance, &IicInstance,
                (XIic_Handler) SendHandler);
    XIic_SetRecvHandler(&IicInstance, &IicInstance,
                (XIic_Handler) ReceiveHandler);

//  xil_printf("i2c slave example begin test2\r\n");
    /*
     * Set the Address as a RESPOND type.
     */
    Status = XIic_SetAddress(&IicInstance, XII_ADDR_TO_RESPOND_TYPE,
                 SLAVE_ADDRESS);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

//  xil_printf("i2c slave example begin test3\r\n");
    /*
     * The IIC Master on this bus should initiate the transfer
     * and write data to the slave at this instance.
     */
//    while(1)
//    {
#if 0
        xil_printf("i2c slave example recv data\r\n");
        SlaveReadData(ReadBuffer, RECEIVE_COUNT);


        delay_SL(0x1fffff);
#endif

//    }
        for (Index = 0; Index < SEND_COUNT; Index++) {
            WriteBuffer[Index] = Index;
        }
#if 0
        xil_printf("i2c slave example write data to master\r\n");
        /*
         * The IIC Master on this bus should initiate the transfer
         * and read data from the slave.
         */
        SlaveWriteData(SEND_COUNT);
#endif

#if 1
    SlaveReadWriteData();
#endif

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function reads a buffer of bytes  when the IIC Master on the bus writes
* data to the slave device.
*
* @param    BufferPtr contains the address of the data buffer to be filled.
* @param    ByteCount contains the number of bytes in the buffer to be read.
*
* @return   XST_SUCCESS if successful else XST_FAILURE.
*
* @note     None
*
******************************************************************************/
int SlaveReadData(u8 *BufferPtr, u16 ByteCount)
{
    int Status;
    int i=0;

    /*
     * Set the defaults.
     */
    ReceiveComplete = 0;

    /*
     * Start the IIC device.
     */
    //xil_printf("i2c slave example begin test4\r\n");
    Status = XIic_Start(&IicInstance);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    /*
     * Set the Global Interrupt Enable.
     */
    XIic_IntrGlobalEnable(IicInstance.BaseAddress);

    //xil_printf("i2c slave example begin test5\r\n");

//  while(1)
//  {
//      XIic_SlaveRecv(&IicInstance, ReadBuffer, RECEIVE_COUNT);

        //delay_SL(0x1fffff);

//  }
    /*
     * Wait for AAS interrupt and completion of data reception.
     */
    while ((ReceiveComplete==0) || (XIic_IsIicBusy(&IicInstance) == TRUE)) {
        if (SlaveRead) {
            XIic_SlaveRecv(&IicInstance, ReadBuffer, RECEIVE_COUNT);
            SlaveRead = 0;
        }
    }
    xil_printf("\r\n slave readbuffer is \r\n");
    for(i=0;i<25;i+=6)
    {
        xil_printf("[%d] 0x%x [%d] 0x%x [%d] 0x%x [%d] 0x%x [%d] 0x%x [%d] 0x%x\r\n",i,ReadBuffer[i],i+1,ReadBuffer[i+1],i+2,ReadBuffer[i+2],i+3,ReadBuffer[i+3],i+4,ReadBuffer[i+4],i+5,ReadBuffer[i+5]);
    }
//  xil_printf("i2c slave example begin test6\r\n");
    /*
     * Disable the Global Interrupt Enable.
     */
    XIic_IntrGlobalDisable(IicInstance.BaseAddress);

//  xil_printf("i2c slave example begin test7\r\n");
    /*
     * Stop the IIC device.
     */
    Status = XIic_Stop(&IicInstance);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function writes a buffer of bytes to the IIC bus when the IIC master
* initiates a read operation.
*
* @param    ByteCount contains the number of bytes in the buffer to be
*       written.
*
* @return   XST_SUCCESS if successful else XST_FAILURE.
*
* @note     None.
*
******************************************************************************/
int SlaveWriteData(u16 ByteCount)
{
    int Status;

    /*
     * Set the defaults.
     */
    TransmitComplete = 0;

    /*
     * Start the IIC device.
     */
    Status = XIic_Start(&IicInstance);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    /*
     * Set the Global Interrupt Enable.
     */
    XIic_IntrGlobalEnable(IicInstance.BaseAddress);

    /*
     * Wait for AAS interrupt and transmission to complete.
     */
    while ((TransmitComplete==0) || (XIic_IsIicBusy(&IicInstance) == TRUE)) {
        if (SlaveWrite) {
            XIic_SlaveSend(&IicInstance, WriteBuffer, SEND_COUNT);
            SlaveWrite = 0;
        }
    }

    /*
     * Disable the Global Interrupt Enable bit.
     */
    XIic_IntrGlobalDisable(IicInstance.BaseAddress);

    /*
     * Stop the IIC device.
     */
    Status = XIic_Stop(&IicInstance);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

int SlaveReadWriteData(void)
{
    int Status;
    ReceiveComplete = 0;
    TransmitComplete = 1;

    Status = XIic_Start(&IicInstance);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    XIic_IntrGlobalEnable(IicInstance.BaseAddress);


//    while ((ReceiveComplete==0) || (XIic_IsIicBusy(&IicInstance) == TRUE)) {
//        if (SlaveRead) {
//            XIic_SlaveRecv(&IicInstance, ReadBuffer, 1);
//            SlaveRead = 0;
//        }
//    }
//
//    xil_printf("slave readbuffer is 0x%x\r\n", ReadBuffer[0]);
//
//    while ((TransmitComplete) || (XIic_IsIicBusy(&IicInstance) == TRUE)) {
//        if (SlaveWrite) {
//            XIic_SlaveSend(&IicInstance, WriteBuffer, SEND_COUNT);
//            SlaveWrite = 0;
//        }
//    }

    while(1)
    {
        if(SlaveReadWrite==1)
        {
//        	XIic_GetStats(&IicInstance, &IicStatus);
            XIic_SlaveRecv(&IicInstance, ReadBuffer, RECEIVE_COUNT);
            if(ReceiveComplete)
            {
                ReceiveComplete = 0;
                SlaveReadWrite = 0;
                SlaveReadBytes = IicInstance.Stats.RecvBytes-IicStatus.RecvBytes;
                if(ReadBuffer[0]==0x5a && (SlaveReadBytes == 1))
                {
                    for (u8 Index = 0; Index < SEND_COUNT; Index++)
                    {
                        WriteBuffer_1[Index] = Index+1;
                    }
                }
            }
        }
        else if(SlaveReadWrite==2)
        {
            if(ReadBuffer[0]==0x5a && (SlaveReadBytes == 1))
            {
                XIic_SlaveSend(&IicInstance, WriteBuffer_1, SEND_COUNT);
            }
            else
            {
                XIic_SlaveSend(&IicInstance, WriteBuffer, SEND_COUNT);
            }
            if(TransmitComplete)
            {
                TransmitComplete = 0;
                SlaveReadWrite = 0;
                SlaveReadBytes = 0;
                memset(ReadBuffer, 0, RECEIVE_COUNT);
            }
        }
    }

    XIic_IntrGlobalDisable(IicInstance.BaseAddress);

    Status = XIic_Stop(&IicInstance);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

/****************************************************************************/
/**
* This Status handler is called asynchronously from an interrupt context and
* indicates the events that have occurred.
*
* @param    InstancePtr is not used, but contains a pointer to the IIC
*       device driver instance which the handler is being called for.
* @param    Event indicates whether it is a request for a write or read.
*
* @return   None.
*
* @note     None.
*
****************************************************************************/
static void StatusHandler(XIic *InstancePtr, int Event)
{
	/*
	 * Check whether the Event is to write or read the data from the slave.
	 */
	if (Event == XII_MASTER_WRITE_EVENT) {
		/*
		 * Its a Write request from Master.
		 */
//		SlaveRead = 1;
		SlaveReadWrite = 1; // MST write to SLV
		XIic_GetStats(&IicInstance, &IicStatus);
	} else {
		/*
		 * Its a Read request from the master.
		 */
//		SlaveWrite = 1;
		SlaveReadWrite = 2; // MST read from SLV
	}
}

/****************************************************************************/
/**
* This Send handler is called asynchronously from an interrupt
* context and indicates that data in the specified buffer has been sent.
*
* @param    InstancePtr is a pointer to the IIC driver instance for which
*       the handler is being called for.
*
* @return   None.
*
* @note     None.
*
****************************************************************************/
static void SendHandler(XIic *InstancePtr)
{
    TransmitComplete = 1;
}

/****************************************************************************/
/**
* This Receive handler is called asynchronously from an interrupt
* context and indicates that data in the specified buffer has been Received.
*
* @param    InstancePtr is a pointer to the IIC driver instance for which
*       the handler is being called for.
*
* @return   None.
*
* @note     None.
*
****************************************************************************/
static void ReceiveHandler(XIic *InstancePtr)
{
    ReceiveComplete = 1;
}

/****************************************************************************/
/**
* This function setups the interrupt system so interrupts can occur for the
* IIC. The function is application-specific since the actual system may or
* may not have an interrupt controller. The IIC device could be directly
* connected to a processor without an interrupt controller. The user should
* modify this function to fit the application.
*
* @param    IicInstPtr contains a pointer to the instance of the IIC  which
*       is going to be connected to the interrupt controller.
*
* @return   XST_SUCCESS if successful else XST_FAILURE.
*
* @note     None.
*
****************************************************************************/
static int SetupInterruptSystem(XIic * IicInstPtr)
{
	int Status;

//#if defined (ARMR5) || (__aarch64__) || (__arm__)
//	XScuGic *IntcInstPtr = &InterruptController;
//#else
//	XIntc *IntcInstPtr = &InterruptController;
//#endif
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	if (InterruptController.IsReady == XIL_COMPONENT_IS_READY) {
		return XST_SUCCESS;
	}
	asm("nop");
#else
	if (InterruptController.IsStarted == XIL_COMPONENT_IS_STARTED) {
		return XST_SUCCESS;
	}
#endif

	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	XScuGic_Config *IntcCfgPtr;

	IntcCfgPtr = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if(IntcCfgPtr == NULL)
	{
		xil_printf("ERR:: Interrupt Controller not found");
		return (XST_DEVICE_NOT_FOUND);
	}
	Status = XScuGic_CfgInitialize(&InterruptController, IntcCfgPtr, IntcCfgPtr->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#else
	Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
					 (Xil_ExceptionHandler) XScuGic_InterruptHandler,
					 &InterruptController);
#else
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				 (Xil_ExceptionHandler) XIntc_InterruptHandler,
				 &InterruptController);
#endif
	/*
	 * Enable non-critical exceptions.
	 */
	Xil_ExceptionEnable();

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above
	 * performs the specific interrupt processing for the device.
	 */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	//为中断设置中断处理函数
	Status = XScuGic_Connect(&InterruptController, IIC_INT_VEC_ID,
				(Xil_ExceptionHandler) XIic_InterruptHandler, (void *) IicInstPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}
#else
	Status = XIntc_Connect(&InterruptController, IIC_INTR_ID,
				   (XInterruptHandler) XIic_InterruptHandler,
				   IicInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif


	/*
	 * Start the interrupt controller so interrupts are enabled for all
	 * devices that cause interrupts.
	 */
#if defined (__MICROBLAZE__)
	Status = XIntc_Start(&InterruptController, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif

	/*
	 * Enable the interrupts for the IIC device.
	 */
#if defined (ARMR5) || (__aarch64__) || (__arm__)
	XScuGic_Enable(&InterruptController, IIC_INT_VEC_ID);
#else
	XIntc_Enable(&InterruptController, IIC_INTR_ID);
#endif

    return XST_SUCCESS;
}
