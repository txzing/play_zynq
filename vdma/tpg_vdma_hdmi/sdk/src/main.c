/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "sleep.h"
#include "platform.h"
#include "xparameters.h"
#include "xil_printf.h"
#include "xgpio.h"
#include "xclk_wiz.h"
#include "xv_tpg.h"
#include "xvidc.h"
#include "xvtc.h"
#include "video_resolution.h"
#include "vtiming_gen.h"
#include "PS_i2c.h"
#include "vdma_api.h"

#define VDMA_ID          		XPAR_AXIVDMA_0_DEVICE_ID
#define VDMA_BASE_ADDR 			0x08000000
#define SCREEN_X				1920
#define SCREEN_Y				1080

#define CLK_LOCK			1
XClk_Wiz ClkWiz_Dynamic; /* The instance of the ClkWiz_Dynamic */

XV_tpg Tpg;
XV_tpg_Config *Tpg_ConfigPtr;
XTpg_PatternId      Pattern;      /**< Video pattern */

XVtc		VtcInst;		/**< Instance of the VTC core. */
XVtc_Config *VtcConfig;

XIicPs IicInstance;
XGpio GpioOutput;

XAxiVdma     vdma;

/*****************************************************************************/
/**
*
* This function generates video pattern.
*
* @param  IsPassThrough specifies either pass-through or colorbar mode.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_ConfigTpg(XV_tpg *InstancePtr)
{
  XV_tpg *pTpg = InstancePtr;

//  XVidC_VideoStream *HdmiTxSsVidStreamPtr;
//  HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(&HdmiTxSs);

//  u32 width, height;
//  XVidC_VideoMode VideoMode;
//  VideoMode = HdmiTxSsVidStreamPtr->VmId;
//
//  if ((VideoMode == XVIDC_VM_1440x480_60_I) ||
//      (VideoMode == XVIDC_VM_1440x576_50_I) )
//  {
//	    //NTSC/PAL Support
//	    width  = HdmiTxSsVidStreamPtr->Timing.HActive/2;
//	    height = HdmiTxSsVidStreamPtr->Timing.VActive;
//  }
//  else {
//	    width  = HdmiTxSsVidStreamPtr->Timing.HActive;
//	    height = HdmiTxSsVidStreamPtr->Timing.VActive;
//  }

  //Stop TPG
  XV_tpg_DisableAutoRestart(pTpg);


  XV_tpg_Set_height(pTpg, 1080);
  xil_printf("the height is %d.\r\n",XV_tpg_Get_height(pTpg));
  XV_tpg_Set_width(pTpg, 1920);
  xil_printf("the width is %d.\r\n",XV_tpg_Get_width(pTpg));

  XV_tpg_Set_colorFormat(pTpg, XVIDC_CSF_RGB);
//  XV_tpg_Set_colorFormat(pTpg, XVIDC_CSF_YCRCB_422);
  XV_tpg_Set_bckgndId(pTpg, XTPG_BKGND_COLOR_BARS);
  //XV_tpg_Set_bckgndId(pTpg, Pattern);
  XV_tpg_Set_ovrlayId(pTpg, 0);

//  XV_tpg_Set_enableInput(pTpg, IsPassThrough);

//  if (IsPassThrough) {
//	  XV_tpg_Set_passthruStartX(pTpg,0);
//	  XV_tpg_Set_passthruStartY(pTpg,0);
//	  XV_tpg_Set_passthruEndX(pTpg,width);
//	  XV_tpg_Set_passthruEndY(pTpg,height);
//  }


  //move box
  XV_tpg_Set_ovrlayId(pTpg, 1);
  XV_tpg_Set_boxSize(pTpg,80);
  //if in YUV mode, G->Y,B->U(Cb),R->V(Cr)
  XV_tpg_Set_boxColorR(pTpg,255);
  XV_tpg_Set_boxColorG(pTpg,255);
  XV_tpg_Set_boxColorB(pTpg,255);
  XV_tpg_Set_motionSpeed(pTpg,1);



  //Start TPG
  XV_tpg_EnableAutoRestart(pTpg);
  XV_tpg_Start(pTpg);

  xil_printf("INFO: TPG configured\r\n");
}

/*****************************************************************************/
/**
*
* This is the Wait_For_Lock function, it will wait for lock to settle change
* frequency value
*
* @param	CfgPtr_Dynamic provides pointer to clock wizard dynamic config
*
* @return
*		- Error 0 for pass scenario
*		- Error > 0 for failure scenario
*
* @note		None
*
******************************************************************************/
int Wait_For_Lock(XClk_Wiz_Config *CfgPtr_Dynamic)
{
	u32 Count = 0;
	u32 Error = 0;

	while(!(*(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK)) {
		if(Count == 10000) {
			Error++;
			break;
		}
		Count++;
        }
    return Error;
}

/*****************************************************************************/
/**
*
* XClk_Wiz wizard dynamic reconfig.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if XClk_Wiz dynamic reconfig ran successfully.
*		- XST_FAILURE if XClk_Wiz dynamic reconfig failed.
*
* @note		None.
*
****************************************************************************/
int XClk_Wiz_dynamic_reconfig()
{
	XClk_Wiz_Config *CfgPtr_Dynamic;
    u32 Count = 0;
    u32 Divide = 0;
    u32 Multiply_Int =0;
    u32 Multiply_Frac =0;
    u32 Divide0_Int = 0;
    u32 Divide0_Frac = 0;
    int Status;

	/*
	 * Get the CLK_WIZ Dynamic reconfiguration driver instance
	 */
	CfgPtr_Dynamic = XClk_Wiz_LookupConfig(XPAR_CLK_WIZ_0_DEVICE_ID);
	if (!CfgPtr_Dynamic) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the CLK_WIZ Dynamic reconfiguration driver
	 */
	Status = XClk_Wiz_CfgInitialize(&ClkWiz_Dynamic, CfgPtr_Dynamic,
		 CfgPtr_Dynamic->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	Status = Wait_For_Lock(CfgPtr_Dynamic);
	if(Status) {
		xil_printf("\n ERROR: Clock is not locked for default frequency" \
	" : 0x%x\n\r", *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK);
	 }

	/* SW reset applied */
	*(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x00) = 0xA;

	if(*(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK) {
		xil_printf("\n ERROR: Clock is locked : 0x%x \t expected "\
	  "0x00\n\r", *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK);
	}

	/* Wait cycles after SW reset */
	for(Count = 0; Count < 2000; Count++);

	Status = Wait_For_Lock(CfgPtr_Dynamic);
	if(Status) {
		  xil_printf("\n ERROR: Clock is not locked after SW reset :"
		  "0x%x \t Expected  : 0x1\n\r",
		  *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK);
	}
	// VCO = freq_in * 37.125 / 4

	Multiply_Int = 37;
	Multiply_Frac = 125;
	Divide = 4;
	/* Configuring Multiply and Divide values */
	*(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x200) = (Multiply_Frac << 16) | (Multiply_Int << 8) | Divide;
	*(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x204) = 0x00;

	// freq_out0 = VCO / 6.250

	Divide0_Int = 6;
	Divide0_Frac = 250;
    /* Configuring Multiply and Divide values */
    *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x208) = (Divide0_Frac << 8) | Divide0_Int;
    *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x20C) = 0x00;

    /* Load Clock Configuration Register values */
    *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x25C) = 0x07;

    if(*(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK) {
        xil_printf("\n ERROR: Clock is locked : 0x%x \t expected "
	    "0x00\n\r", *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK);
     }

	/* Clock Configuration Registers are used for dynamic reconfiguration */
	*(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x25C) = 0x02;

	Status = Wait_For_Lock(CfgPtr_Dynamic);
    if(Status)
    {
        xil_printf("\n ERROR: Clock is not locked : 0x%x \t Expected "\
        		": 0x1\n\r", *(u32 *)(CfgPtr_Dynamic->BaseAddr + 0x04) & CLK_LOCK);
    }

	return XST_SUCCESS;
}


int main()
{
	uint32_t			status;

    init_platform();

//    printf("Please wait...\n");
    xil_printf("Please wait...\n\r");

    status = XClk_Wiz_dynamic_reconfig();
	if (status != XST_SUCCESS)
	{
		xil_printf("XClk_Wiz dynamic reconfig failed.\r\n");
		return XST_FAILURE;
	}
	xil_printf("XClk_Wiz dynamic reconfig ok\n\r");


	xil_printf("TPG Initializing \r\n");
	Tpg_ConfigPtr = XV_tpg_LookupConfig(XPAR_V_TPG_0_DEVICE_ID);
    if(Tpg_ConfigPtr == NULL)
    {
    	Tpg.IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }

    status = XV_tpg_CfgInitialize(&Tpg,
				Tpg_ConfigPtr, Tpg_ConfigPtr->BaseAddress);
    if(status != XST_SUCCESS)
    {
        xil_printf("ERR:: TPG Initialization failed %d\r\n", status);
        return(XST_FAILURE);
    }
    xil_printf("TPG Initialization ok\r\n");

    XV_ConfigTpg(&Tpg);





    VtcConfig = XVtc_LookupConfig(XPAR_VTC_0_DEVICE_ID);
    status = XVtc_CfgInitialize(&VtcInst, VtcConfig, VtcConfig->BaseAddress);
    if(status != XST_SUCCESS)
	{
		xil_printf("VTC Initialization failed %d\r\n", status);
		return(XST_FAILURE);
	}

    vtiming_gen_run(&VtcInst, VIDEO_RESOLUTION_1080P, 2);


    i2c_init(&IicInstance, XPAR_XIICPS_0_DEVICE_ID,100000);
    XGpio_Initialize(&GpioOutput, XPAR_AXI_GPIO_0_DEVICE_ID);   //initialize GPIO IP
    XGpio_SetDataDirection(&GpioOutput, 1, 0x0);            //set GPIO as output
	XGpio_DiscreteWrite(&GpioOutput, 1, 0x0);               //set GPIO output value to 0
	usleep(50000);
	XGpio_DiscreteWrite(&GpioOutput, 1, 0x1);
	i2c_reg8_write(&IicInstance,0x72>>1,0x08,0x35);
	i2c_reg8_write(&IicInstance,0x7a>>1,0x2f,0x00);
	// sil9134 in yuv422 out rgb
	//i2c_reg8_write(&IicInstance,0x72>>1,0x48,0x30); // csc select
	//i2c_reg8_write(&IicInstance,0x72>>1,0x4a,0x3c);

	// sil9134 in yuv422 out yuv422
	//i2c_reg8_write(&IicInstance,0x72>>1,0x48,0x20);
	//i2c_reg8_write(&IicInstance,0x72>>1,0x4a,0x00);
	
	// sil9134 in yuv422 out yuv444
	//i2c_reg8_write(&IicInstance,0x72>>1,0x48,0x20);
	//i2c_reg8_write(&IicInstance,0x72>>1,0x4a,0x14);
	
    // sil9134 in yuv444 out rgb
	//i2c_reg8_write(&IicInstance,0x72>>1,0x48,0x10);
	//i2c_reg8_write(&IicInstance,0x72>>1,0x4a,0x38);

	run_vdma_frame_buffer(&vdma, VDMA_ID, SCREEN_X, SCREEN_Y,
				VDMA_BASE_ADDR,0,0,BOTH);

//    cleanup_platform();
    return 0;
}
