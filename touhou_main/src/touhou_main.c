#include "debug_funcs.h"
#include "helper.h"
#include "intr.h"
#include "platform.h"
#include "xil_printf.h"
#include <stdint.h>
int main() {
    init_platform();
    xil_printf("\r\nTouhou ECE385, Version:" __DATE__ " " __TIME__ "\r\n");
    Xil_SetTlbAttributes(0xFFFF0000,0x14de2);
//    print("ARM0: writing startaddress for ARM1\n\r");
    // Write the memory space base address in the Zynq's DDR (PS7 DDR) for
    // ARM core 1 to 0xFFFFFFF0 (which is 0x10080000 in this project).
    // cf. https://www.hackster.io/whitney-knitter/dual-arm-hello-world-on-zynq-using-vitis-9fc8b7
    Xil_Out32(0xFFFFFFF0, 0x3F000000);
    dmb(); //waits until write has finished
//    print("ARM0: sending the SEV to wake up ARM1\n\r");
    __asm__("sev");
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
    clear_bullet();
    int32_t bullet_x = 0;
    int32_t bullet_y = 0;
    int32_t dir_x = 1;
    int32_t dir_y = 1;

    uint32_t game_state = 1;
    static uint8_t start_col = 0;
    int32_t player_x = 0;
    int32_t player_y = 0;
    uint32_t player_hp = 24;
    uint32_t enemy_hp = 5000;
    uint32_t fall_down = 0;
    uint32_t fb1_alt_disp = 0;
    // frame loop/ game loop
    uint32_t bul_type[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    setMemFlag(INFORM_READER,1);
    sleep(1);
    while (1) {
        switch (game_state) {
        case 0:
            go_menu();
            draw_text(FB1_BASE, 240, 380, RGB(0x66 + start_col, 0xCC + start_col, 0xFF + start_col), "Press Z to start");
            start_col++;
            get_keys();
            if (key_z) {
                game_state = 1;
                clear_text(FB1_BASE, 240, 380, 20);
                setup_AUDIO(BGM, BGM_STAGE6_PATH);
            }
            break;
        case 1: // playing stage
        	setMemFlag(INFORM_READER,1);
            get_keys();
            if (key_left) {
                player_x -= 5;
            }
            if (key_right) {
                player_x += 5;
            }
            if (key_up) {
                player_y -= 5;
            }
            if (key_down) {
                player_y += 5;
            }
            if (key_z) {
                setup_AUDIO(SFX, SFX4_PL_SHOOT);
            }
            if (player_x < 0) {
                player_x = 0;
            }
            if (player_x > 384 - 32) {
                player_x = 384 - 32;
            }
            if (player_y < 0) {
                player_y = 0;
            }
            if (player_y > 448 - 48) {
                player_y = 448 - 48;
            }
            // set bullet(hardware)
            for (int i = 0; i < 6; i++) {
                set_enemy_bullet(i, 32 * i, fall_down, bul_type[i]);
            }
            // set_player_bullet(0, bullet_x, bullet_y, 0);

            time_tick_last = get_time_tick();
            toggle_render();
            Xil_WaitForEventSet(XSCUGIC_SW_TIMEOUT_VAL, 1, &InitNewFrameCond);
            InitNewFrameCond = FALSE;
            time_tick = get_time_tick();
            printf("2Drt:%f ms \r\n",((float)time_tick_last-(float)time_tick)/(float)333333);
            // check detect collision before drawing software content
            volatile uint32_t *ptr;
            if (!fb1_alt_disp) {
                // check FB1_ALT_BASE
                ptr = (uint32_t *)FB1_ALT_BASE;
            } else {
                // check FB1_BASE
                ptr = (uint32_t *)FB1_BASE;
            }
            // 32x48 player box
            uint32_t cur_val = ptr[FB1_START_X + player_x + 16 + 640 * (FB1_START_Y + player_y + 24)];
            if (cur_val & 0x2) {
                player_hp -= 2;
                set_die_buzzer();
                // xil_printf("Player hit!\r\n");
                if (player_hp == 0) {
                    game_state = 2;
                    setup_AUDIO(BGM, BGM_DEAD);
                    clear_text(FB1_ALT_BASE, 640 - 15 * 8, 64, 14);
                    clear_text(FB1_BASE, 640 - 15 * 8, 64, 14);
                    clear_text(FB1_ALT_BASE, 640 - 15 * 8, 48, 14);
                    clear_text(FB1_BASE, 640 - 15 * 8, 48, 14);
                }
            } else {
                clear_die_buzzer();
            }
            // do software drawing
            // player and enemy

           char player_hp_str[14];
           char enemy_hp_str[14];
           sprintf(player_hp_str, "Player HP:%d", player_hp);
           sprintf(enemy_hp_str, "Enemy HP:%d", enemy_hp / 10);
           if (fb1_alt_disp) {
//                soft_draw_board_sprite(FB1_BASE, PLAYER, 0, player_x, player_y);
//                soft_draw_board_sprite(FB1_BASE, ENEMY, 2, (384 - 32) / 2, 0);
               fb1_alt_disp = 0;

               // draw Text

               clear_text(FB1_BASE, 640 - 15 * 8, 48, 14);
               clear_text(FB1_BASE, 640 - 15 * 8, 64, 14);
               draw_text(FB1_BASE, 640 - 15 * 8, 48, RGB(0xFF, 0, 0), player_hp_str);
               draw_text(FB1_BASE, 640 - 15 * 8, 64, RGB(0, 0, 0xFF), enemy_hp_str);
           } else {
//                soft_draw_board_sprite(FB1_ALT_BASE, PLAYER, 0, player_x, player_y);
//                soft_draw_board_sprite(FB1_ALT_BASE, ENEMY, 2, (384 - 32) / 2, 0);
               fb1_alt_disp = 1;
               // draw Text
               clear_text(FB1_ALT_BASE, 640 - 15 * 8, 48, 14);
               clear_text(FB1_ALT_BASE, 640 - 15 * 8, 64, 14);
               draw_text(FB1_ALT_BASE, 640 - 15 * 8, 48, RGB(0xFF, 0, 0), player_hp_str);
               draw_text(FB1_ALT_BASE, 640 - 15 * 8, 64, RGB(0, 0, 0xFF), enemy_hp_str);
           }
            // toggle fb1 alt
            toggle_fb1_alt();

            // update bullet position
            fall_down += 1;
            if (fall_down > 448) {
                fall_down = 0;
                for (int i = 0; i < 12; i++) {
                    // new random bullet type
                    bul_type[i] = rand() % 16;
                }
            }
            if (enemy_hp == 0) {
                game_state = 3;
                setup_AUDIO(BGM, BGM_START);
                clear_text(FB1_ALT_BASE, 640 - 15 * 8, 64, 14);
                clear_text(FB1_BASE, 640 - 15 * 8, 64, 14);
                clear_text(FB1_ALT_BASE, 640 - 15 * 8, 48, 14);
                clear_text(FB1_BASE, 640 - 15 * 8, 48, 14);
                setup_AUDIO(SFX, SFX13_ENEMY_DEAD);
            }
            enemy_hp--;
            break;
        case 2: // bad ending
            if (!fb1_alt_disp) {
                draw_text(FB1_BASE, 240, 240, RGB(0xFF, 0xFF, 0xFF), "You lose!");
            } else {
                draw_text(FB1_ALT_BASE, 240, 240, RGB(0xFF, 0xFF, 0xFF), "You lose!");
            }

            break;

        case 3: // good ending
            if (!fb1_alt_disp) {
                draw_text(FB1_BASE, 240, 240, RGB(0xFF, 0xFF, 0xFF), "You win!");
            } else {
                draw_text(FB1_ALT_BASE, 240, 240, RGB(0xFF, 0xFF, 0xFF), "You win!");
            }
            break;
        }
    }

    cleanup_platform();
    return 0;
}
