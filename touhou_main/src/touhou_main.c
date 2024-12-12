#include "debug_funcs.h"
#include "helper.h"
#include "platform.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include <stdio.h>
// #define TOUHOU1
// #define TOUHOU2
// #define ZJU_SHOW
#define SLEEP
int main() {
    init_platform();
    xil_printf("\r\nTest Touhou board, Version:" __DATE__ " " __TIME__ "\r\n");
    // GPIO Init
    uint32_t volume = 5; // max 7,1/128 default 1/8
    uint32_t bgm = 0;
    XGpioPs_Config *cfg = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
    XGpioPs led;
    XGpioPs_CfgInitialize(&led, cfg, cfg->BaseAddr);
    XGpioPs_SetDirection(&led, 2,
                         0xFFFFFFFF); // 1 for output, set bank2 as all output
    XGpioPs_SetDirection(&led, 3,
                         0x00000000); // 0 for input, set bank3 as all input
    XGpioPs_SetDirection(&led, 0,
                         0xFFFFFFFF); // 1 for output, set bank0 as all output
    // Test PS GPIO
    GPIO0_OUT |= 0x1 << 7; // set led mio7 to 1
                           // Clear Framebuffer
    clear_fb0();
    clear_fb0_alt();
    clear_fb1();
    clear_fb1_alt();
    // load SD content to DDR3
    SDIO_Read(FB0_BASE, 0, NUM_BLOCKS);
    // Clear buzzer
    clear_die_buzzer();
    // Setup BGM
    //  set_audio_volume(volume);
    //  setup_AUDIO(BGM,bgm);
    // Test keyboard read
    //  test_keys();
    // Test control
    //  debug_console();

    uint32_t sfx_type = 0;
    uint32_t bgm_type = 0;
    while (1) {
        draw_board_color(FB1_BASE, RGB(255, 0, 0));
        usleep(1000000); // 1s
        draw_board_color(FB1_BASE, RGB(255, 0, 0));
        usleep(1000000); // 1s
    }

    cleanup_platform();
    return 0;
}