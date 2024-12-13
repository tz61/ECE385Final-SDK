#include "debug_funcs.h"
#include "helper.h"
#include "intr.h"
#include "platform.h"
#include "xil_printf.h"
// #define TOUHOU1
// #define TOUHOU2
// #define ZJU_SHOW
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
    // SDIO_Read(FB0_BASE, 0, NUM_BLOCKS);
    // Clear buzzer
    clear_die_buzzer();
    // Setup BGM
    set_audio_volume(volume);
    setup_AUDIO(BGM, bgm);
    // setup interrupt
    setup_irq();
    // Test keyboard read
    //  test_keys();
    // Test control
    //  debug_console();

    copy_bullet_sprite_to_dest();
    uint32_t sfx_type = 0;
    uint32_t bgm_type = 0;
    // init timer
    setup_timer();
    // Done Init everything
    // draw_board_color(FB1_ALT_BASE, RGB(0x66, 0xCC, 0xFF));
    // draw_board_color(FB1_BASE, RGB(255, 255, 255));

    uint32_t time_tick = 0, time_tick_last = 0;
    uint32_t frame = 0;
    time_tick = get_time_tick();
    clear_enemy_bullet();
    clear_player_bullet();
    map_enemy_bullet_to_vram();
    map_player_bullet_to_vram();
    int32_t bullet_x = 0;
    int32_t bullet_y = 0;
    int32_t dir_x = 1;
    int32_t dir_y = 1;
    while (1) {
        // frame loop
        // ReadAnimation();
        // go_menu();
        ReadAnimation();
        set_enemy_bullet(0, bullet_x, bullet_y, 15);
        map_enemy_bullet_to_vram();

        set_player_bullet(0, bullet_x, bullet_y, 2);
        map_player_bullet_to_vram();
        test_draw2d_time(); // 4ms, max 16ms
        // test_60frame_time();

        // usleep(1000000); // 1s

        // test_bullet_map(FB1_BASE);
        // uint32_t ms = (time_tick_last - time_tick) / 333333;
        // xil_printf("Draw Time:%x, to msecond:%d, curtime:%x\r\n", time_tick_last - time_tick, ms, time_tick);
        // time_tick_last = time_tick;
        // usleep(1000000); // 1s
        // draw_board_color(FB1_BASE, RGB(255, 255, 255)); takes around 88ms
        // usleep(1000000); // 1s
        // draw_board_strips(FB1_ALT_BASE);
    }

    cleanup_platform();
    return 0;
}
