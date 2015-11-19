#include "Talkthrough.h"
#include "filter.h"
#include <string.h>


//extern fir_state_fr16 s_left;
//extern fir_state_fr16 s_right;


//--------------------------------------------------------------------------//
// Function:	Process_Data()												//
//																			//
// Description: This function is called for each DMA RX Complete Interrupt, //
//				or 2*FRAMESIZE samples for a stereo signal. Then left and   //
//				right channels are separately filtered ping-pong mode.	    //
//--------------------------------------------------------------------------//
void Process_Data(void)
{

	// Ping-Pong Flag
    static int ping = 0;

	// Buffers
		bufferedframe f_l[3], f_r[3];
		bufferedframe e_l[3], e_r[3];
		for (int i = 0; i < 3; i++) {
			f_l[i%3]->nextframe=&f_l[(i+1)%3];
			f_r[i%3]->nextframe=&f_r[(i+1)%3];
			e_l[i%3]->nextframe=&e_l[(i+1)%3];
			e_r[i%3]->nextframe=&e_r[(i+1)%3];
		}
		static int last_l=0;
		static int last_r=0;
		static int PTlast_l=0;
		static int PTlast_r=0;

    /* core processing in ping-pong mode */
    if(0 == ping) {


        // left and right channels processing, ping slot
        slowdown(RxPing+0, TxPing+0, &f_l[2], &e_l[2], FRAMESIZE,2,last_l,PTlast_l);
        slowdown(RxPing+1, TxPing+1, framebuf_right+fb_r, excbuf_right+eb_r, FRAMESIZE,2,last_r,PTlast_r);

//        memcpy(TxPing, RxPing, 2*FRAMESIZE*sizeof(RxPing[0]));

    } else {


        // left and right channels processing, pong slot
        slowdown(RxPong+0, TxPong+0, framebuf_left+fb_l, excbuf_left+eb_l, FRAMESIZE,2,last_l,PTlast_l);
        slowdown(RxPong+1, TxPong+1, framebuf_right+fb_r, excbuf_right+eb_r, FRAMESIZE,2,last_r,PTlast_r);

//        memcpy(TxPong, RxPong, 2*FRAMESIZE*sizeof(RxPong[0]));

    }


/*
    static int cnt = 0;
    cnt = cnt + 1;

    if((cnt>=100) && (cnt<140))
    {
        if (0==ping)
        {
     		memcpy((void *)(RxBUF+2*FRAMESIZE*(cnt-100)),RxPing,2*FRAMESIZE*sizeof(RxPing[0]));
     		memcpy((void *)(TxBUF+2*FRAMESIZE*(cnt-100)),TxPing,2*FRAMESIZE*sizeof(TxPing[0]));
        }
     	else
     	{
     		memcpy((void *)(RxBUF+2*FRAMESIZE*(cnt-100)),RxPong,2*FRAMESIZE*sizeof(RxPong[0]));
     		memcpy((void *)(TxBUF+2*FRAMESIZE*(cnt-100)),TxPong,2*FRAMESIZE*sizeof(TxPong[0]));
     	}
    }
*/
    ping ^= 0x1;

}
