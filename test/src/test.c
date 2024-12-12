#include <stdio.h>
#include "xgpiops.h"
#include "xsdps.h"
#include "platform.h"
#include "xil_printf.h"

#define DDR_TEST_PTR 0x200000
#define DDR_TEST_END 0x40000000
#define FB0_BASE 0x01000000
#define FB1_BASE 0x0112c000
volatile uint32_t* sddest_ptr=0x01000000;

//#define NUM_BLOCKS (491520+524288)//Two volumes
#define NUM_BLOCKS (9600)//Two volumes
/* Sector offset to test */
#define SECTOR_OFFSET 0
static int SdpsRawTest(void)
{
	static XSdPs SdInstance;
	XSdPs_Config *SdConfig;
	int Status;
	u32 BuffCnt;
	/*
	 * Since block size is 512 bytes. File Size is 512*BlockCount.
	 */
	u32 FileSize = (512 * NUM_BLOCKS); /* File Size is only up to 2MB */
	u32 Sector = SECTOR_OFFSET;


	/*
	 * Initialize the host controller
	 */
#ifndef SDT
	SdConfig = XSdPs_LookupConfig(XPAR_XSDPS_0_DEVICE_ID);
#else
	SdConfig = XSdPs_LookupConfig(XPAR_XSDPS_0_BASEADDR);
#endif
	if (NULL == SdConfig) {
		return XST_FAILURE;
	}

	Status = XSdPs_CfgInitialize(&SdInstance, SdConfig,
				     SdConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSdPs_CardInitialize(&SdInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	if (!(SdInstance.HCS)) {
		Sector *= XSDPS_BLK_SIZE_512_MASK;
	}


	/*
	 * Read data from SD/eMMC.
	 */
	Status  = XSdPs_ReadPolled(&SdInstance, Sector, NUM_BLOCKS,
			sddest_ptr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	return XST_SUCCESS;
}
int main()
{
    init_platform();
    volatile uint32_t* ptr;
    uint32_t cnt=0;
    uint32_t error=0;
    XGpioPs_Config *cfg= XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
    XGpioPs led;
    XGpioPs_CfgInitialize(&led,cfg,cfg->BaseAddr);
    XGpioPs_SetDirectionPin(&led,7,1);//1 for output
    SdpsRawTest();
    xil_printf("\r\nTest DDR @ZYNQ7010 @DDR3L 533.3333MHz!\n\r Build with gcc " __VERSION__ " at " __DATE__ " " __TIME__ " !\r\n");
    while(1){
    	XGpioPs_WritePin(&led,7,1);
    	ptr = FB0_BASE;
    	  Xil_DCacheDisable();
    	  for (int i = 0; i < 480; i++) {
    	    for (int j = 0; j < 640; j++) {
//    	      *ptr = 0xFFFF0000;
    	      //  xil_printf("%x",board[i*640+j]);
    	      ptr++;
    	    }
    	  }
    	  Xil_DCacheEnable();
    	  XGpioPs_WritePin(&led,7,0);
    	  ptr= FB1_BASE;

    	  Xil_DCacheDisable();
    	  for (int i = 0; i < 480; i++) {
    	    for (int j = 0; j < 640; j++) {
//    	      *ptr = 0x0000FF00;
    	      //  xil_printf("%x",board[i*640+j]);
    	      ptr++;
    	    }
    	  }
    	  Xil_DCacheEnable();
    }
    for(uint32_t i = DDR_TEST_PTR;i<DDR_TEST_END;i+=0x04){
        ptr = i;
        *ptr = i;
        if(i% 0x1000000 == 0){
            xil_printf("Write at addr 0x%x!\t",i);
            cnt++;
            if(cnt==4){
            	xil_printf("\r\n");
              	cnt=0;
            }
        }
        //        xil_printf("Write to address at 0x%x, value:0x%x\r\n",i,i);
    }
    for(uint32_t i = DDR_TEST_PTR;i<DDR_TEST_END;i+=0x04){
        ptr = i;
//        xil_printf("address at 0x%x, value:0x%x\r\n",i,*ptr);
        if(i% 0x1000000 == 0){
          	xil_printf("Read at addr 0x%x!\t",i);
           	cnt++;
           	if(cnt==4){
           		xil_printf("\r\n");
          		cnt=0;
          	}
      	}
        if(i!=*ptr){
           	xil_printf("mismatch at addr 0x%x!\r\n",i);
        }
    }
    if(!error){
    	xil_printf("\r\nDDR3L check done:1GByte no mismatch!\r\n");
    }
    cleanup_platform();
    return 0;
}
