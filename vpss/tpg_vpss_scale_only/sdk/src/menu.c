// SPDX-License-Identifier: GPL-2.0
/*
 * menu.c
 *
 *  Created on: 18 March 2019
 *      Author: florentw
 */
 
 #include "menu.h"
 
void display_main_menu()
{
	 xil_printf("\r\n-----------------------------\r\n");
	 xil_printf("Select an option:\r\n");
	 xil_printf("1. Change TPG background\r\n");
	 xil_printf("2. Enable/Disable TPG moving box\r\n");
	 xil_printf("3. Change input resolution\r\n");
	 xil_printf("4. Change output resolution\r\n");
	 xil_printf("5. Show VPSS log\r\n");
	 xil_printf("6. AXI4-Stream to Video Out status\r\n");
	 xil_printf("z. Display this menu again\r\n");
	 xil_printf("\r\n-----------------------------\r\n");
}

int tpg_menu()
{
	char userInput;

	xil_printf("\r\n-----------------------------\r\n");
	xil_printf("Change Background pattern:\r\n");
	xil_printf("1. Horizontal Ramp\r\n");
	xil_printf("2. Vertical Ramp\r\n");
	xil_printf("3. Temporal Ramp\r\n");
	xil_printf("4. Solid Red\r\n");
	xil_printf("5. Solid Blue\r\n");
	xil_printf("6. Solid Green\r\n");
	xil_printf("7. Solid Black\r\n");
	xil_printf("8. Solid White\r\n");
	xil_printf("9. Color Bar\r\n");
	xil_printf("\r\n-----------------------------\r\n");

	userInput = inbyte();

	if((userInput >= '1')&&(userInput <= '9'))
	{
		return (userInput - 48);
	}
	else
	{
		xil_printf("Invalid Option\r\n");
		return 10;
	}
}

int resolution_menu(int direction)
{
	int i;
	char userInput;

	xil_printf("\r\n-----------------------------\r\n");
	if(direction)
		xil_printf("Select the new input resolution:\r\n");
	else
		xil_printf("Select the new output resolution:\r\n");

	for(i=0;i<=SUPPORTED_VIDEO_FORMATS-1;i++)
	{
		xil_printf("%d. ",i);
		print_resolution_name(i);
		xil_printf("\r\n");
	}
	xil_printf("\r\n-----------------------------\r\n");

	userInput = inbyte();

	if((userInput >= '0')&&(userInput <= '8'))
	{
		return (userInput - 48);
	}
	else
	{
		xil_printf("Invalid Option\r\n");
		return 10;
	}
}

void menu(app_periphs_t *periphs_ptr)
{
	char userInput;

	display_main_menu();
	userInput = inbyte();
	int selection;

	switch(userInput)
	{
		case '1':
			selection = tpg_menu();
			if(selection !=0)
			{
				periphs_ptr->tpg_config.bckgndId=selection;
				configure_tpg(periphs_ptr->tpg_ptr, &periphs_ptr->tpg_config);
			}
			break;
		case '2':
			if(periphs_ptr->tpg_config.overlay_en)
			{
				periphs_ptr->tpg_config.overlay_en=0;
				xil_printf("Overlay disabled\r\n");
			}
			else
			{
				periphs_ptr->tpg_config.overlay_en=1;
				xil_printf("Overlay enabled\r\n");
			}
			configure_tpg(periphs_ptr->tpg_ptr, &periphs_ptr->tpg_config);
			break;
		case '3':
			selection = resolution_menu(1);
			if(selection !=10)
			{
				set_input_resolution(periphs_ptr, selection);
			}
			break;
		case '4':
			selection = resolution_menu(0);
			if(selection !=10)
			{
				set_output_resolution(periphs_ptr, selection);
			}
			break;
		case '5':
			display_vpss_log(periphs_ptr->Vproc_ptr);
			break;
		case '6':
			display_axi4vidout_sts(XPAR_AXI_GPIO_0_BASEADDR);
			break;
		case 'z':
			display_main_menu();
			break;
		default:
			xil_printf("\nWrong Command\r\n");
			break;
	}
}
