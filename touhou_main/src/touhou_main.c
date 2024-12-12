#include "platform.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include <stdio.h>
#include "helper.h"
#include "debug_funcs.h"
//#define TOUHOU1
//#define TOUHOU2
//#define ZJU_SHOW
#define SLEEP
int main() {
  init_platform();
  xil_printf("\r\nTest Touhou board, Version:" __DATE__ " " __TIME__ "\r\n");
  // GPIO Init
  volatile uint32_t *ptr;
  uint32_t volume=5;//max 7,1/128 default 1/8
  uint32_t bgm=0;
  uint8_t red, green, blue;
  uint32_t pix_data;
  XGpioPs_Config *cfg= XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
  XGpioPs led;
  XGpioPs_CfgInitialize(&led,cfg,cfg->BaseAddr);
  XGpioPs_SetDirection(&led,2,0xFFFFFFFF);//1 for output, set bank2 as all output
  XGpioPs_SetDirection(&led,3,0x00000000);//0 for input, set bank3 as all input
  XGpioPs_SetDirection(&led,0,0xFFFFFFFF);//1 for output, set bank0 as all output
  // Test PS GPIO
  GPIO0_OUT |= 0x1<<7; // set led mio7 to 1
  // Clear Framebuffer
//  clear_fb0();
//  clear_fb0_alt();
//  clear_fb1();
//  clear_fb1_alt();
  // load SD content to DDR3
  SDIO_Read(FB0_BASE,0,NUM_BLOCKS);
  // Setup BGM
//  set_audio_volume(volume);
//  setup_AUDIO(BGM,bgm);
  // Test keyboard read
//  test_keys();

  while(1){
	  xil_printf("GPIO3:input:%x\r\n",GPIO3_IN);
	  xil_printf("Pleinput new volume:\r\n");
	  scanf("%d",&volume);
//
//	  xil_printf("input new BGM:\r\n");
//	  scanf("%d",&bgm);
//	  xil_printf("set new volume:%d\r\n",volume);
//	  xil_printf("set new BGM:%d\r\n",bgm);
	  set_audio_volume(volume);
	  setup_AUDIO(BGM,bgm);
//	  toggle_fb0_alt();
//	  if(GPIO3_IN & (0x1<<1)){//bit 0 done, bit 1 idle, bit 2 ready
	  if(volume==10){//bit 0 done, bit 1 idle, bit 2 ready
		  xil_printf("Issue new draw!\r\n");
		  toggle_render();
	  }else if(volume==1){
		  toggle_fb0_alt();
	  }else if(volume==2){
		  set_die_buzzer();
	  }else if(volume==3){
		  clear_die_buzzer();
	  }

	  usleep(1000000);
//	  clear_fb0();
	  usleep(1000000);
  }
  // Populate audio
  ptr = AUDIO_BASE_ADDR;
  uint32_t sfx_type=0;
  uint32_t bgm_type=0;
  while (1) {

    // clear screen


#ifdef SLEEP
    usleep(DELAY_TIME);
#endif
    ptr = FB1_BASE;
    Xil_DCacheDisable();
    for (int i = FB1_START_Y; i < FB1_END_Y; i++) {
      ptr = i * LINE_STRIDE_BYTE + FB1_BASE + FB1_START_X * 4;
      for (int j = FB1_START_X; j < FB1_END_X; j++) {
        *ptr = 0xFF000001;
        //  xil_printf("%x",board[i*640+j]);
        ptr++;
      }
    }
    ptr--;
    pix_data = *ptr;
    Xil_DCacheEnable();
    xil_printf("Red Done,lastpixel:%x\r\n", pix_data);

#ifdef SLEEP
    usleep(DELAY_TIME);
#endif
    ptr = FB1_BASE;
    Xil_DCacheDisable();
    for (int i = FB1_START_Y; i < FB1_END_Y; i++) {
      ptr = i * LINE_STRIDE_BYTE + FB1_BASE + FB1_START_X * 4;
      red = 25;
      green = 25;
      blue = 25;
      for (int j = FB1_START_X; j < FB1_END_X; j++) {
        red = (red + 1) % 256;
        green = (green + j / 3) % 256;
        blue = (blue + j / 5) % 256;
        *ptr = (red << 24) | (green << 16) | (blue << 8) | (1);
        //    		*ptr = 0x12345678;
        ptr += 0x1;
      }
    }
    ptr--;
    pix_data = *ptr;
    Xil_DCacheEnable();
    xil_printf("Strip Done,lastpixel:%x\r\n", pix_data);
#ifdef SLEEP
    usleep(DELAY_TIME);
#endif
    // ====================Here FB1_ALT====================

    // clear screen
    ptr = FB1_ALT_BASE;
    Xil_DCacheDisable();
    for (int i = FB1_START_Y; i < FB1_END_Y; i++) {
      ptr = i * LINE_STRIDE_BYTE + FB1_ALT_BASE + FB1_START_X * 4;
      for (int j = FB1_START_X; j < FB1_END_X; j++) {
        *ptr = 0x00FF0001;
        //    		xil_printf("%x",board[i*640+j]);
        ptr++;
      }
    }
    ptr--;
    pix_data = *ptr;
    Xil_DCacheEnable();
    xil_printf("Blue Done,lastpixel:%x:\r\n", pix_data);

#ifdef SLEEP
    usleep(DELAY_TIME);
#endif
    ptr = FB1_ALT_BASE;
    Xil_DCacheDisable();
    for (int i = FB1_START_Y; i < FB1_END_Y; i++) {
      ptr = i * LINE_STRIDE_BYTE + FB1_ALT_BASE + FB1_START_X * 4;
      for (int j = FB1_START_X; j < FB1_END_X; j++) {
        *ptr = 0xFF000001;
        //  xil_printf("%x",board[i*640+j]);
        ptr++;
      }
    }
    ptr--;
    pix_data = *ptr;
    Xil_DCacheEnable();
    xil_printf("Red Done,lastpixel:%x\r\n", pix_data);
    ptr = FB1_ALT_BASE;
    Xil_DCacheDisable();
    for (int i = FB1_START_Y; i < FB1_END_Y; i++) {
      ptr = i * LINE_STRIDE_BYTE + FB1_ALT_BASE + FB1_START_X * 4;
      red = 25;
      green = 25;
      blue = 25;
      for (int j = FB1_START_X; j < FB1_END_X; j++) {
        red = (red + 1) % 256;
        green = (green + j / 3) % 256;
        blue = (blue + j / 5) % 256;
        *ptr = (red << 24) | (green << 16) | (blue << 8) | (1);
        //    		*ptr = 0x12345678;
        ptr += 0x1;
      }
    }
    ptr--;
    pix_data = *ptr;
    Xil_DCacheEnable();
    xil_printf("Strip Done,lastpixel:%x\r\n", pix_data);
#ifdef SLEEP
    usleep(DELAY_TIME);
#endif
  }
  cleanup_platform();
  return 0;
}
