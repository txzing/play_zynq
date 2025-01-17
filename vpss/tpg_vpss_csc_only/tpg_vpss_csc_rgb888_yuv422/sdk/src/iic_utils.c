// SPDX-License-Identifier: GPL-2.0
/*
 * iic_utils.c
 *
 *  Created on: 23 Nov 2018
 *      Author: Florent Werbrouck
 */

#include "iic_utils.h"

int ps_iic_init(u16 DeviceId, XIicPs* IicPs)
{
	int Status;
	XIicPs_Config *Config;

	Config = XIicPs_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(IicPs, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Set the IIC serial clock rate.
	Status = XIicPs_SetSClk(IicPs, IIC_SCLK_RATE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

void set_iic_mux( XIicPs *IicPs, u8 MuxSelect, u8 Address)
{
	u8 data = MuxSelect;
	iic_write( IicPs, Address, data, 1 );
}

int iic_write( XIicPs *IicPs, u8 Address, u8 Data, s32 ByteCount)
{
	int Status;

	// Wait until bus is idle to start another transfer.
	while (XIicPs_BusIsBusy(IicPs)) {
		/* NOP */
	}

	// Send the buffer using the IIC and check for errors.
	Status = XIicPs_MasterSendPolled(IicPs, &Data, ByteCount, Address);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

int iic_write2( XIicPs *IicPs, u8 Address, u8 Register, u8 Data )
{
	u8 WriteBuffer[2];
	int Status;

	/* A temporary write buffer must be used which contains both the address
	 * and the data to be written, put the address in first */
	WriteBuffer[0] = Register;
	WriteBuffer[1] = Data;


	// Wait until bus is idle to start another transfer.

	while (XIicPs_BusIsBusy(IicPs)) {
		/* NOP */
	}

	// Send the buffer using the IIC and check for errors.
	Status = XIicPs_MasterSendPolled(IicPs, WriteBuffer, 2, Address);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

int iic_read( XIicPs *IicPs, u8 Address, u8 Register, u8 *Data, int ByteCount)
{

	int Status;

	// Wait until bus is idle to start another transfer.
	while (XIicPs_BusIsBusy(IicPs)) {
		/* NOP */
	}

	// Set the IIC Repeated Start option.
	Status = XIicPs_SetOptions(IicPs, XIICPS_REP_START_OPTION);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	// Set the address of the register to read
	Status = XIicPs_MasterSendPolled(IicPs, &Register, 1, Address);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Receive the data.
	Status = XIicPs_MasterRecvPolled(IicPs, Data, ByteCount, Address);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Clear the IIC Repeated Start option.
	Status = XIicPs_ClearOptions(IicPs, XIICPS_REP_START_OPTION);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

//int check_hdmi_hpd_status(XIicPs *IicPs, u8 Address)
//{
//	int Status;
//	u8 data = 0x00;
//
//	//
//	Status = iic_read(IicPs, Address, ADV7511_HDP_REG_ADDR, &data, 1);
//	// Status = iic_read2( IicPs, /*ZC702_HDMI_ADDR*/ 0x39, 0x42, &data, 1);
//	if (Status != XST_SUCCESS) {
//		return XST_FAILURE;
//	}
//
//	if((data & ADV7511_HPD_CTRL_MASK) == ADV7511_HPD_CTRL_MASK) {
//		// Monitor Connected
//		return 1;
//	}
//	else
//	{
//		// Monitor not connected
//		return 0;
//	}
//}

//void configure_adv7511(XIicPs *IicPs, u8 Address)
//{
//
//	// Power-up the Tx
//	iic_write2(IicPs, Address, 0x41,0x10);
//
//
//	// Fixed registers that must be set on power up
//	iic_write2(IicPs, Address, 0x98, 0x03);
//	iic_write2(IicPs, Address, 0x9A, 0xE0);
//	iic_write2(IicPs, Address, 0x9C, 0x30);
//	iic_write2(IicPs, Address, 0x9D, 0x61);
//	iic_write2(IicPs, Address, 0xA2, 0xA4);
//	iic_write2(IicPs, Address, 0xA3, 0xA4);
//	iic_write2(IicPs, Address, 0xE0, 0xD0);
//	iic_write2(IicPs, Address, 0xF9, 0x00);
//
//	// Color Space Conversion
//	iic_write2(IicPs, Address, 0x18, 0xE7); //CSC Enabled (YCbCr to RGB)
//
//	// Set RGB in AVinfo Frame
//	iic_write2(IicPs, Address, 0x55, 0x00);
//	// Aspect Ration
//	iic_write2(IicPs, Address, 0x56, 0x28);
//
//	// HPD Control always high
//	iic_write2(IicPs, Address, 0xD6, 0xC0);
//
//	// DVI Mode, no HDCP
//	iic_write2(IicPs, Address, 0xAF, 0x04);
//
//	// Fixed I2C Address
//	iic_write2(IicPs, Address, 0xF9, 0x00);
//
//}
//
///*This function set the parameters specific for the ZC702*/
//void configure_adv7511_zc702(XIicPs *IicPs, u8 Address)
//{
//    // ADV7511 Input / Output Mode
//    //YCbCr 422 with Separated Syncs
//    iic_write2(IicPs, Address, 0x15, 0x01);
//    //YCbCr444 Output Format, Style 1, 8bpc
//    iic_write2(IicPs, Address, 0x16, 0x38);
//    //Video Input Justification: Right justified
//    iic_write2(IicPs, Address, 0x48, 0x08);
//}
//
///*This wait for a monitor to be connected to the ADV7511*/
//void wait_for_monitor(XIicPs *IicPs, u8 Address)
//{
//    //Check the state of the HPD signal
//	u8 monitor_connected = 0;
//	u8 temp = 2;
//    while(!monitor_connected)
//    {
//    	monitor_connected = check_hdmi_hpd_status(IicPs, Address);
//
//    	if(monitor_connected != temp)
//    	{
//    		temp = monitor_connected;
//    		if(monitor_connected)
//    		{
//    			xil_printf("HDMI Monitor connected\r\n");
//    		}
//    		else
//    		{
//    			xil_printf("No HDMI Monitor connected / Monitor Disconnected\r\n");
//    		}
//    		sleep(2);
//    	}
//    }
//}

void set_on_board_hdmi(XIicPs *IicPs, XGpio *XGpio_reset)
{
    //Configure the PS IIC Controller
    ps_iic_init(XPAR_XIICPS_0_DEVICE_ID, IicPs);


    XGpio_Initialize(XGpio_reset, XPAR_AXI_GPIO_0_DEVICE_ID);   //initialize GPIO IP
    XGpio_SetDataDirection(XGpio_reset, 1, 0x0);            //set GPIO as output
	XGpio_DiscreteWrite(XGpio_reset, 1, 0x0);               //set GPIO output value to 0
	usleep(50000);
	XGpio_DiscreteWrite(XGpio_reset, 1, 0x1);

    // Set the iic mux to the ADV7511
//    set_iic_mux(IicPs, ZC702_I2C_SELECT_HDMI, ZC702_I2C_MUX_ADDR);

    //Wait for the monitor to be connected
//    wait_for_monitor(IicPs, ZC702_HDMI_ADDR);

    // ADV7511 Basic Configuration
//    configure_adv7511(IicPs,ZC702_HDMI_ADDR);
    // ADV7511 ZC702 Specific configuration
//    configure_adv7511_zc702(IicPs,ZC702_HDMI_ADDR);

	iic_write2(IicPs,0x72>>1,0x08,0x35);
	iic_write2(IicPs,0x7a>>1,0x2f,0x00);

	// sil9134 in yuv422 out rgb
	iic_write2(IicPs,0x72>>1,0x48,0x30); // csc select
	iic_write2(IicPs,0x72>>1,0x4a,0x3c);

	// sil9134 in yuv422 out yuv422
	//iic_write2(IicPs,0x72>>1,0x48,0x20);
	//iic_write2(IicPs,0x72>>1,0x4a,0x00);
	
	// sil9134 in yuv422 out yuv444
	//iic_write2(IicPs,0x72>>1,0x48,0x20);
	//iic_write2(IicPs,0x72>>1,0x4a,0x14);
}
