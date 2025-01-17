// SPDX-License-Identifier: GPL-2.0
/*
 * app_hdmi.c
 *
 *  Created on: 07 Jan 2018
 *      Author: Florent Werbrouck
 */

#include "app_hdmi.h"
#include "vdma_api.h"

/* Peripheral IP driver Instance */
XIicPs IicPs_inst;
XV_tpg tpg_inst;
XVtc VtcInst;
XVprocSs VprocInst;
XGpio GpioInst;
XClk_Wiz ClkWizInst;
XAxiVdma     vdma;

const VideoTimingClk_t VideoTimingClk[SUPPORTED_VIDEO_FORMATS]=
{
		{XVIDC_VM_1920x1080_60_P, 	CLKWIZ_CLKOUT0_148_5_MHz},
		{XVIDC_VM_1280x1024_60_P,	CLKWIZ_CLKOUT0_108_MHz},
		{XVIDC_VM_1280x960_60_P,	CLKWIZ_CLKOUT0_108_MHz},
		{XVIDC_VM_1280x720_60_P, 	CLKWIZ_CLKOUT0_74_25_MHz},
		{XVIDC_VM_1024x768_60_P,	CLKWIZ_CLKOUT0_65_MHz},
		{XVIDC_VM_800x600_60_P,		CLKWIZ_CLKOUT0_40_MHz},
		{XVIDC_VM_640x480_60_P,		CLKWIZ_CLKOUT0_25_175MHz}
};

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function configures the TPG core.
* @param
* @param
*
*
******************************************************************************/
void configure_tpg(XV_tpg *tpg_ptr, tpg_config_t *tpg_config)
{
	// Set Resolution
    XV_tpg_Set_height(tpg_ptr, tpg_config->height);
    XV_tpg_Set_width(tpg_ptr, tpg_config->width);

    // Set Color Space
    XV_tpg_Set_colorFormat(tpg_ptr, tpg_config->colorFormat);

    // Change the pattern to color bar
    XV_tpg_Set_bckgndId(tpg_ptr, tpg_config->bckgndId);

    if(tpg_config->overlay_en)
    {
    	// Set Overlay to moving box
		// Set the size of the box
		XV_tpg_Set_boxSize(tpg_ptr, tpg_config->boxSize);
		// Set the speed of the box
		XV_tpg_Set_motionSpeed(tpg_ptr, tpg_config->motionSpeed);
    }

    XV_tpg_Set_ovrlayId(tpg_ptr, tpg_config->overlay_en);

}

/*****************************************************************************/
/**
*
* This function configures the VPSS core.
* @param
* @param
*
*
******************************************************************************/
int configure_vpss(XVprocSs *InstancePtr, const XVidC_VideoStream *StrmIn,
					const XVidC_VideoStream *StrmOut)
{
	int Status;

	XVprocSs_SetVidStreamIn(InstancePtr, StrmIn);

	XVprocSs_SetVidStreamOut(InstancePtr, StrmOut);

	Status = XVprocSs_SetSubsystemConfig(InstancePtr);

	return Status;
}

/*****************************************************************************/
/**
*
* This function configures the VDMA core.
* @param
* @param
*
*
******************************************************************************/
void configure_vdma(UINTPTR BaseAddr,const XVidC_VideoStream *Strm, u32 bytePerPixels)
{

	u32 width, height;

	width = Strm->Timing.HActive;
	height = Strm->Timing.VActive;

	/* Start of VDMA Configuration */
    /* Configure the Write interface (S2MM)*/
    // S2MM Control Register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x30, 0x8B);
    //S2MM Start Address 1
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xAC, FRAME_BUFFER_1);
    //S2MM Start Address 2
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xB0, FRAME_BUFFER_2);
    //S2MM Start Address 3
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xB4, FRAME_BUFFER_3);
    //S2MM Frame delay / Stride register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xA8, width*bytePerPixels);
    // S2MM HSIZE register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xA4, width*bytePerPixels);
    // S2MM VSIZE register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0xA0, height);

    /* Configure the Read interface (MM2S)*/
    // MM2S Control Register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x00, 0x8B);
    // MM2S Start Address 1
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x5C, FRAME_BUFFER_1);
    // MM2S Start Address 2
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x60, FRAME_BUFFER_2);
    // MM2S Start Address 3
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x64, FRAME_BUFFER_3);
    // MM2S Frame delay / Stride register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x58, width*bytePerPixels);
    // MM2S HSIZE register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x54, width*bytePerPixels);
    // MM2S VSIZE register
    Xil_Out32(XPAR_AXI_VDMA_0_BASEADDR + 0x50, height);

//    run_vdma_frame_buffer(&vdma, XPAR_AXIVDMA_0_DEVICE_ID, width, height,
//    		FRAME_BUFFER_BASE_ADDR,0,0,BOTH);
	xil_printf("VDMA started!\r\n");
}

/*****************************************************************************/
/**
*
* This function configures the VTC Generator core.
* @param
* @param
*
*
******************************************************************************/
void configure_vtc_gen(XVtc *InstancePtr, const XVidC_VideoStream *Strm)
{
	XVtc_Reset(InstancePtr);
	XVtc_DisableGenerator(InstancePtr);
	XVtc_Disable(InstancePtr);

	XVtc_Timing XVtc_Timingconf;

	XVtc_Timingconf.HActiveVideo 	= Strm->Timing.HActive;
	XVtc_Timingconf.HBackPorch		= Strm->Timing.HBackPorch;
	XVtc_Timingconf.HFrontPorch		= Strm->Timing.HFrontPorch;
	XVtc_Timingconf.HSyncPolarity	= Strm->Timing.HSyncPolarity;
	XVtc_Timingconf.HSyncWidth		= Strm->Timing.HSyncWidth;
	XVtc_Timingconf.Interlaced		= Strm->IsInterlaced;
	XVtc_Timingconf.V0BackPorch		= Strm->Timing.F0PVBackPorch;
	XVtc_Timingconf.V0FrontPorch	= Strm->Timing.F0PVFrontPorch;
	XVtc_Timingconf.V0SyncWidth		= Strm->Timing.F0PVSyncWidth;
	XVtc_Timingconf.V1BackPorch		= Strm->Timing.F1VBackPorch;
	XVtc_Timingconf.V1FrontPorch	= Strm->Timing.F1VFrontPorch;
	XVtc_Timingconf.V1SyncWidth		= Strm->Timing.F1VSyncWidth;
	XVtc_Timingconf.VActiveVideo	= Strm->Timing.VActive;
	XVtc_Timingconf.VSyncPolarity	= Strm->Timing.VSyncPolarity;

	//Configure the VTC
	//XVtc_ConvVideoMode2Timing(&VtcInst,XVTC_VMODE_576P,&XVtc_Timingconf);
	XVtc_SetGeneratorTiming(&VtcInst, &XVtc_Timingconf);
	XVtc_RegUpdate(InstancePtr);

	//Start the VTC generator
	XVtc_Enable(InstancePtr);
	XVtc_EnableGenerator(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function configures the input stream.
* @param
* @param
*
*
******************************************************************************/
void set_input_resolution(app_periphs_t *periphs_ptr, int resID)
{

	const XVidC_VideoTimingMode *VmPtr ;

	//Set Stream In
	VmPtr=XVidC_GetVideoModeData(VideoTimingClk[resID].VmId);
	periphs_ptr->video_pipe_config.Stream_in.Timing = VmPtr->Timing;
	periphs_ptr->video_pipe_config.Stream_in.VmId = VmPtr->VmId;
	periphs_ptr->video_pipe_config.Stream_in.ColorFormatId = APP_COLOR_FORMAT;
	periphs_ptr->video_pipe_config.Stream_in.ColorDepth = periphs_ptr->Vproc_ptr->Config.ColorDepth;
	periphs_ptr->video_pipe_config.Stream_in.PixPerClk = periphs_ptr->Vproc_ptr->Config.PixPerClock;
	periphs_ptr->video_pipe_config.Stream_in.FrameRate = VmPtr->FrameRate;
	periphs_ptr->video_pipe_config.Stream_in.IsInterlaced = 0;

	//Reset the video pipe


	/* Clocking Wizard Configuration */
	//Configure the CLKOUT0 DIV
	//ClkWiz_Set_Output_Clock(XPAR_CLK_WIZ_0_BASEADDR, VideoTimingClk[resID].clkFreq);
	/* End of clocking wizard configuration */

	//Configure the TPG
	XV_tpg_DisableAutoRestart(periphs_ptr->tpg_ptr);
	periphs_ptr->tpg_config.height = periphs_ptr->video_pipe_config.Stream_in.Timing.VActive;
	periphs_ptr->tpg_config.width = periphs_ptr->video_pipe_config.Stream_in.Timing.HActive;

//	u32 Data;
//    Data = XV_tpg_ReadReg(periphs_ptr->tpg_ptr->Config.BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL) & 0x80;
//    XV_tpg_WriteReg(periphs_ptr->tpg_ptr->Config.BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL, Data &~ 0x01);
	usleep(50000);
	configure_vpss(periphs_ptr->Vproc_ptr, &periphs_ptr->video_pipe_config.Stream_in,
					&periphs_ptr->video_pipe_config.Stream_out);

//	usleep(50000);
	configure_tpg(periphs_ptr->tpg_ptr, &periphs_ptr->tpg_config);
	//Start the TPG
	XV_tpg_EnableAutoRestart(periphs_ptr->tpg_ptr);
	usleep(50000);
	XV_tpg_Start(periphs_ptr->tpg_ptr);
//	XV_tpg_Start(periphs_ptr->tpg_ptr);

	configure_vdma(XPAR_AXI_VDMA_0_BASEADDR,&periphs_ptr->video_pipe_config.Stream_out, 3);
	configure_vtc_gen(periphs_ptr->Vtc_ptr,&periphs_ptr->video_pipe_config.Stream_out);
}

/*****************************************************************************/
/**
*
* This function configures the out stream.
* @param
* @param
*
*
******************************************************************************/
void set_output_resolution(app_periphs_t *periphs_ptr, int resID)
{

	const XVidC_VideoTimingMode *VmPtr ;

	//Set Stream Out
	VmPtr=XVidC_GetVideoModeData(VideoTimingClk[resID].VmId);
	periphs_ptr->video_pipe_config.Stream_out.Timing = VmPtr->Timing;
	periphs_ptr->video_pipe_config.Stream_out.VmId = VmPtr->VmId;
	periphs_ptr->video_pipe_config.Stream_out.ColorFormatId = APP_COLOR_FORMAT;
	periphs_ptr->video_pipe_config.Stream_out.ColorDepth = periphs_ptr->Vproc_ptr->Config.ColorDepth;
	periphs_ptr->video_pipe_config.Stream_out.PixPerClk = periphs_ptr->Vproc_ptr->Config.PixPerClock;
	periphs_ptr->video_pipe_config.Stream_out.FrameRate = VmPtr->FrameRate;
	periphs_ptr->video_pipe_config.Stream_out.IsInterlaced = 0;

	/* Clocking Wizard Configuration */
	//Configure the CLKOUT0 DIV
	ClkWiz_Set_Output_Clock(XPAR_CLK_WIZ_0_BASEADDR, VideoTimingClk[resID].clkFreq);
	/* End of clocking wizard configuration */

	configure_vpss(periphs_ptr->Vproc_ptr, &periphs_ptr->video_pipe_config.Stream_in,
					&periphs_ptr->video_pipe_config.Stream_out);

	configure_tpg(periphs_ptr->tpg_ptr, &periphs_ptr->tpg_config);
	//Start the TPG
	XV_tpg_EnableAutoRestart(periphs_ptr->tpg_ptr);
	XV_tpg_Start(periphs_ptr->tpg_ptr);

	configure_vdma(XPAR_AXI_VDMA_0_BASEADDR,&periphs_ptr->video_pipe_config.Stream_out, 3);
	configure_vtc_gen(periphs_ptr->Vtc_ptr,&periphs_ptr->video_pipe_config.Stream_out);

}

/****************************************************************************/
/**
*
*	This function wait for some time and check if the clocking wizard is locked
*
*
* @param	outClockFreq frequency to be set.
*
* @return	status.
*
* @note		None.
*
******************************************************************************/
int ClkWiz_Wait_For_Lock(UINTPTR Addr)
{
	usleep(800);
	if(Xil_In32(Addr + 0x04)& CLK_LOCK)
	{
		return XST_SUCCESS;
	}
	else
	{
		return XST_FAILURE;
	}
}


/*****************************************************************************/
/**
*
* This function configure the clock output of the clocking wizard
* @param
* @param
* @param
*
*
******************************************************************************/
int ClkWiz_Set_Output_Clock(UINTPTR ClkWiz_BaseAddr, u8 outClockFreq)
{
	int status;

	//Soft Reset is required from 2018.3
	Xil_Out32(ClkWiz_BaseAddr, 0xA);
	usleep(800);

	switch(outClockFreq)
	{
		case CLKWIZ_CLKOUT0_148_5_MHz:
			//Set clock to 148.5 MHz
			// [7:0] = DIVCLK_DIVIDE = 8 (0x8)
			// [15:8] = CLKFBOUT_MULT = 37 (0x25)
			// [25:16] = CLKFBOUT_FRAC = 125 (0x7D)
//			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x7D2508);
			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x7D2504);
			// DIV = 6.250 (0x06.0xFA)
			Xil_Out32(ClkWiz_BaseAddr + 0x208, 0xFA06);
			// Update the clocking wizard
			Xil_Out32(ClkWiz_BaseAddr + 0x25C, 0x3);
			break;

		case CLKWIZ_CLKOUT0_108_MHz:
			//Set clock to 108 MHz
			// [7:0] = DIVCLK_DIVIDE = 2 (0x2)
			// [15:8] = CLKFBOUT_MULT = 10 (0x0A)
			// [25:16] = CLKFBOUT_FRAC = 125 (0x7D)
//			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x7D0A02);
			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x7D0A01);
			// DIV = 9.375 (0x09.0x177)
			Xil_Out32(ClkWiz_BaseAddr + 0x208, 0x17709);
			// Update the clocking wizard
			Xil_Out32(ClkWiz_BaseAddr + 0x25C, 0x3);
			break;

		case CLKWIZ_CLKOUT0_74_25_MHz:
			//Set clock to 74.25 MHz
			// [7:0] = DIVCLK_DIVIDE = 8 (0x8)
			// [15:8] = CLKFBOUT_MULT = 37 (0x25)
			// [25:16] = CLKFBOUT_FRAC = 125 (0x7D)
//			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x7D2508);
			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x7D2504);
			// DIV = 12.500 (0x0C.0x1FA)
			Xil_Out32(ClkWiz_BaseAddr + 0x208, 0x1FA0C);
			// Update the clocking wizard
			Xil_Out32(ClkWiz_BaseAddr + 0x25C, 0x3);
			break;

		case CLKWIZ_CLKOUT0_74_MHz:
			//Set clock to 74 MHz
			// [7:0] = DIVCLK_DIVIDE = 10 (0xA)
			// [15:8] = CLKFBOUT_MULT = 50 (0x32)
			// [25:16] = CLKFBOUT_FRAC = 875 (0x36B)
//			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x36B320A);
			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x36B3205);
			// DIV = 13.750 (0x0D.0x2EE)
			Xil_Out32(ClkWiz_BaseAddr + 0x208, 0x2EE0D);
			// Update the clocking wizard
			Xil_Out32(ClkWiz_BaseAddr + 0x25C, 0x3);
			break;

		case CLKWIZ_CLKOUT0_65_MHz:
			//Set clock to 65 MHz
			// [7:0] = DIVCLK_DIVIDE = 10 (0x0A)
			// [15:8] = CLKFBOUT_MULT = 50 (0x32)
			// [25:16] = CLKFBOUT_FRAC = 375 (0x177)
//			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x177320A);
			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x1773205);
			// DIV = 15.500 (0x0F.0x1F4)
			Xil_Out32(ClkWiz_BaseAddr + 0x208, 0x1F40F);
			// Update the clocking wizard
			Xil_Out32(ClkWiz_BaseAddr + 0x25C, 0x3);
			break;

		case CLKWIZ_CLKOUT0_40_MHz:
			//Set clock to 40 MHz
			// [7:0] = DIVCLK_DIVIDE = 1 (0x01)
			// [15:8] = CLKFBOUT_MULT = 5 (0x05)
			// [25:16] = CLKFBOUT_FRAC = 0 (0x0)
//			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x501);
			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0xA01);
			// DIV = 25.000 (0x19.0x00)
			Xil_Out32(ClkWiz_BaseAddr + 0x208, 0x19);
			// Update the clocking wizard
			Xil_Out32(ClkWiz_BaseAddr + 0x25C, 0x3);
			break;

		case CLKWIZ_CLKOUT0_27_MHz:
			//Set clock to 27 MHz
			// [7:0] = DIVCLK_DIVIDE = 2 (0x02)
			// [15:8] = CLKFBOUT_MULT = 10 (0xA)
			// [25:16] = CLKFBOUT_FRAC = 125 (0x7D)
//			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x7D0A02);
			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x7D0A01);
			// DIV = 37.500 (0x25.0x1F4)
			Xil_Out32(ClkWiz_BaseAddr + 0x208, 0x1F425);
			// Update the clocking wizard
			Xil_Out32(ClkWiz_BaseAddr + 0x25C, 0x3);
			break;
		case CLKWIZ_CLKOUT0_25_175MHz:
			//Set clock to 27 MHz
			// [7:0] = DIVCLK_DIVIDE = 7 (0x07)
			// [15:8] = CLKFBOUT_MULT = 38 (0x26)
			// [25:16] = CLKFBOUT_FRAC = 0 (0x7D)
//			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x002607);
			Xil_Out32(ClkWiz_BaseAddr + 0x200, 0x7D1B02);
			// DIV = 43.125 (0x2B.0x07D)
//			Xil_Out32(ClkWiz_BaseAddr + 0x208, 0x07D2B);
			Xil_Out32(ClkWiz_BaseAddr + 0x208, 0x36B35);
			// Update the clocking wizard
			Xil_Out32(ClkWiz_BaseAddr + 0x25C, 0x3);
			break;
		default:
			return XST_FAILURE;
			break;
	}

	status = ClkWiz_Wait_For_Lock(ClkWiz_BaseAddr);
	return status;

}

/*****************************************************************************/
/**
*
* This function configure initialize the peripherals
* @param
* @param
* @param
*
*
******************************************************************************/
int init_periphs(app_periphs_t *periphs_ptr)
{
	int Status;
	XVprocSs_Config *VprocSsConfigPtr;

	periphs_ptr->IicPs_ptr 	= &IicPs_inst;
	periphs_ptr->tpg_ptr 	= &tpg_inst;
	periphs_ptr->Vtc_ptr 	= &VtcInst;
	periphs_ptr->Vproc_ptr	= &VprocInst;
	periphs_ptr->Gpio_ptr	= &GpioInst;
//	periphs_ptr->ClkWiz_ptr	= &ClkWizInst;

//	//Initialize the ClkWiz
//    Status = XClk_Wiz_dynamic_reconfig(periphs_ptr->ClkWiz_ptr);
//    if (Status != XST_SUCCESS)
//    {
//      xil_printf("XClk_Wiz dynamic reconfig failed.\r\n");
//      return XST_FAILURE;
//    }
//    xil_printf("XClk_Wiz dynamic reconfig ok\n\r");

	//Configure the ZC702 On-board HDMI
	set_on_board_hdmi(periphs_ptr->IicPs_ptr, periphs_ptr->Gpio_ptr);

	//Initialize the TPG IP
	Status = XV_tpg_Initialize(periphs_ptr->tpg_ptr, XPAR_V_TPG_0_DEVICE_ID);
	if(Status!= XST_SUCCESS)
	{
		xil_printf("TPG configuration failed\r\n");
		return(XST_FAILURE);
	}

	// Initialise the VTC
	XVtc_Config *VTC_Config = XVtc_LookupConfig(XPAR_V_TC_0_DEVICE_ID);
	XVtc_CfgInitialize(periphs_ptr->Vtc_ptr, VTC_Config, VTC_Config->BaseAddress);

	//Configure the VPSS
	VprocSsConfigPtr = XVprocSs_LookupConfig(XPAR_V_PROC_SS_0_DEVICE_ID);
	if(VprocSsConfigPtr == NULL)
	{
		xil_printf("ERR:: VprocSs device not found\r\n");
		return (XST_DEVICE_NOT_FOUND);
	}

	XVprocSs_LogReset(periphs_ptr->Vproc_ptr);
	Status = XVprocSs_CfgInitialize(periphs_ptr->Vproc_ptr,
									VprocSsConfigPtr,
									VprocSsConfigPtr->BaseAddress);
	if(Status != XST_SUCCESS)
	{
		 xil_printf("ERR:: Video Processing Subsystem Init. error\n\r");
		 return(XST_FAILURE);
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function configure initialize the application
* @param
* @param
* @param
*
*
******************************************************************************/
int init_application(app_periphs_t *periphs_ptr)
{
	//int Status;
	const XVidC_VideoTimingMode *VmPtrIn, *VmPtrOut ;

	//Set Stream In
//	VmPtrIn=XVidC_GetVideoModeData(XVIDC_VM_1920x1080_60_P);
	VmPtrIn=XVidC_GetVideoModeData(XVIDC_VM_640x480_60_P);
	periphs_ptr->video_pipe_config.Stream_in.Timing = VmPtrIn->Timing;
	periphs_ptr->video_pipe_config.Stream_in.VmId = VmPtrIn->VmId;
	periphs_ptr->video_pipe_config.Stream_in.ColorFormatId = APP_COLOR_FORMAT;
	periphs_ptr->video_pipe_config.Stream_in.ColorDepth = periphs_ptr->Vproc_ptr->Config.ColorDepth;
	periphs_ptr->video_pipe_config.Stream_in.PixPerClk = periphs_ptr->Vproc_ptr->Config.PixPerClock;
	periphs_ptr->video_pipe_config.Stream_in.FrameRate = VmPtrIn->FrameRate;
	periphs_ptr->video_pipe_config.Stream_in.IsInterlaced = 0;

	//Set Stream Out
//	VmPtrOut=XVidC_GetVideoModeData(XVIDC_VM_640x480_60_P);
	VmPtrOut=XVidC_GetVideoModeData(XVIDC_VM_1920x1080_60_P);
	periphs_ptr->video_pipe_config.Stream_out.Timing = VmPtrOut->Timing;
	periphs_ptr->video_pipe_config.Stream_out.VmId = VmPtrOut->VmId;
	periphs_ptr->video_pipe_config.Stream_out.ColorFormatId = APP_COLOR_FORMAT;
	periphs_ptr->video_pipe_config.Stream_out.ColorDepth = periphs_ptr->Vproc_ptr->Config.ColorDepth;
	periphs_ptr->video_pipe_config.Stream_out.PixPerClk = periphs_ptr->Vproc_ptr->Config.PixPerClock;
	periphs_ptr->video_pipe_config.Stream_out.FrameRate = VmPtrOut->FrameRate;
	periphs_ptr->video_pipe_config.Stream_out.IsInterlaced = 0;

	/* Clocking Wizard Configuration */
	//Configure the CLKOUT0 DIV
	ClkWiz_Set_Output_Clock(XPAR_CLK_WIZ_0_BASEADDR, CLKWIZ_CLKOUT0_148_5_MHz);
	/* End of clocking wizard configuration */

	//Configure the TPG
	periphs_ptr->tpg_config.colorFormat=periphs_ptr->video_pipe_config.Stream_in.ColorFormatId;
	periphs_ptr->tpg_config.bckgndId=XTPG_BKGND_COLOR_BARS;
	periphs_ptr->tpg_config.overlay_en=1;
	periphs_ptr->tpg_config.motionSpeed=1;
	periphs_ptr->tpg_config.boxSize=50;
	periphs_ptr->tpg_config.height = periphs_ptr->video_pipe_config.Stream_in.Timing.VActive;
	periphs_ptr->tpg_config.width = periphs_ptr->video_pipe_config.Stream_in.Timing.HActive;

	xil_printf("Starting VPSS...");
		configure_vpss(periphs_ptr->Vproc_ptr, &periphs_ptr->video_pipe_config.Stream_in,
						&periphs_ptr->video_pipe_config.Stream_out);
	xil_printf("Done!\r\n");

	//Start the TPG
	xil_printf("Starting TPG...");
	configure_tpg(periphs_ptr->tpg_ptr, &periphs_ptr->tpg_config);
	XV_tpg_EnableAutoRestart(periphs_ptr->tpg_ptr);
	XV_tpg_Start(periphs_ptr->tpg_ptr);
	xil_printf("Done!\r\n");

	xil_printf("Starting VDMA...");
	configure_vdma(XPAR_AXI_VDMA_0_BASEADDR,&periphs_ptr->video_pipe_config.Stream_out, 3);
	xil_printf("Done!\r\n");

	xil_printf("Starting VTC...");
	configure_vtc_gen(periphs_ptr->Vtc_ptr,&periphs_ptr->video_pipe_config.Stream_out);
	xil_printf("Done!\r\n");

	return 1;
}

/*****************************************************************************/
/**
*
* This function shows the VPSS log
* @param
* @param
*
*
******************************************************************************/
void display_vpss_log(XVprocSs *InstancePtr)
{

	XVprocSs_LogDisplay(InstancePtr);
	XVprocSs_LogReset(InstancePtr);
}

/*****************************************************************************/
/**
*
* This function print the resolution name
* @param
* @param
*
*
******************************************************************************/
void print_resolution_name(int resID)
{
	xil_printf("%s",XVidC_GetVideoModeStr(VideoTimingClk[resID].VmId));
}

/*****************************************************************************/
/**
*
* This function displays the AXI4-Stream to video out status
* @param
* @param
*
*
******************************************************************************/
void display_axi4vidout_sts(UINTPTR Addr)
{
	u32 status;
	status = Xil_In32(Addr);

	xil_printf("Status [0:12]: 0x%x \r\n",status&0x1FFF);
	xil_printf("Locked: %d \r\n",(status&0x2000)>>13);
	xil_printf("Underflow: %d \r\n",(status&0x4000)>>14);
}
