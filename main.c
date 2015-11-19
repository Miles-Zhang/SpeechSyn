//--------------------------------------------------------------------------//
//																			//
//	 Name: 	Talkthrough with FIR for the ADSP-BF533 EZ-KIT Lite				//
//																			//
//--------------------------------------------------------------------------//
//																			//
//	(C) Copyright 2003 - Analog Devices, Inc.  All rights reserved.			//
//																			//
//	Project Name:	BF533 C Talkthrough TDM									//
//																			//
//	Date Modified:	04/03/03		HD		Rev 1.0							//
//																			//
//	Software:		VisualDSP++3.1											//
//																			//
//	Hardware:		ADSP-BF533 EZ-KIT Board									//
//																			//
//	Connections:	Connect an input source (such as a radio) to the Audio	//
//					input jack and an output source (such as headphones) to //
//					the Audio output jack									//
//																			//
//	Purpose:		This program sets up the SPI port on the ADSP-BF533 to  //
//					configure the AD1836 codec.  The SPI port is disabled 	//
//					after initialization.  The data to/from the codec are 	//
//					transfered over SPORT0 in TDM mode						//
//																			//
//--------------------------------------------------------------------------//

#include "Talkthrough.h"
#include "sysreg.h"
#include "ccblkfn.h"

// FIR stuffs 
#include "mds_def.h"
#include "filter.h"
#include "fir_coeff.h"

//--------------------------------------------------------------------------//
// Variables																//
//																			//
// Description:	The variables iChannelxLeftIn and iChannelxRightIn contain 	//
//				the data coming from the codec AD1836.  The (processed)		//
//				playback data are written into the variables 				//
//				iChannelxLeftOut and iChannelxRightOut respectively, which 	//
//				are then sent back to the codec in the SPORT0 ISR.  		//
//				The values in the array iCodec1836TxRegs can be modified to //
//				set up the codec in different configurations according to   //
//				the AD1885 data sheet.										//
//--------------------------------------------------------------------------//

volatile short sCodec1836TxRegs[CODEC_1836_REGS_LENGTH] =
{									
					DAC_CONTROL_1	| 0x010,
					DAC_CONTROL_2	| 0x000,
					DAC_VOLUME_0	| 0x3ff,
					DAC_VOLUME_1	| 0x3ff,
					DAC_VOLUME_2	| 0x3ff,
					DAC_VOLUME_3	| 0x3ff,
					DAC_VOLUME_4	| 0x3ff,
					DAC_VOLUME_5	| 0x3ff,
					ADC_CONTROL_1	| 0x000,
					ADC_CONTROL_2	| 0x020,
					ADC_CONTROL_3	| 0x000
					
};


/**************************************************
    DMA RX and TX Ping-Pong Buffer Definitions  
***************************************************/
// SPORT0 DMA Receive Double Buffer, ping + pong
short RxBuffer[2*FRAMESIZE + 2*FRAMESIZE];
// SPORT0 DMA Transmit Double Buffer, ping + pong
short TxBuffer[2*FRAMESIZE + 2*FRAMESIZE];
// Ping Pong Buffer Pointers
short* RxPing = RxBuffer;
short* RxPong = RxBuffer + 2*FRAMESIZE;
short* TxPing = TxBuffer;
short* TxPong = TxBuffer + 2*FRAMESIZE;


/*************************************************
    FIR  Definitions
**************************************************/
#define DELAY_SIZE       BASE_TAPLENGTH

// delay buffers for left and right channels
fract16 delay_left[DELAY_SIZE];
fract16 delay_right[DELAY_SIZE];

// two instansces for left and right channel FIRs
fir_state_fr16 s_left;
fir_state_fr16 s_right;

short RxBUF[80*FRAMESIZE];
short TxBUF[80*FRAMESIZE];

//--------------------------------------------------------------------------//
// Function:	main														//
//																			//
// Description:	After calling a few initalization routines, main() just 	//
//				waits in a loop forever.  The code to process the incoming  //
//				data can be placed in the function Process_Data() in the 	//
//				file "Process_Data.c".										//
//--------------------------------------------------------------------------//
void main(void)
{

	sysreg_write(reg_SYSCFG, 0x32);		//Initialize System Configuration Register
	Init_EBIU();
	Init_Flash();
	Init1836();
	Init_Sport0();
	Init_DMA();
	Init_Sport_Interrupts();
	Enable_DMA_Sport0();

	// FIR initializations
    fir_init(s_left, h, delay_left, BASE_TAPLENGTH);	
    fir_init(s_right, h, delay_right, BASE_TAPLENGTH);	
	
	while(1);
}
