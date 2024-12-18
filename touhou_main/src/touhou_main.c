#include "debug_funcs.h"
#include "helper.h"
#include "intr.h"
#include "platform.h"
#include "xil_printf.h"
#include <math.h>
#include <stdint.h>
#define min(a, b) (((a) < (b)) ? (a) : (b))
int main() {
    init_platform();
    xil_printf("\r\nTouhou ECE385, Version:" __DATE__ " " __TIME__ "\r\n");
    Xil_SetTlbAttributes(0xFFFF0000, 0x14de2);
    //    print("ARM0: writing startaddress for ARM1\n\r");
    // Write the memory space base address in the Zynq's DDR (PS7 DDR) for
    // ARM core 1 to 0xFFFFFFF0 (which is 0x10080000 in this project).
    // cf. https://www.hackster.io/whitney-knitter/dual-arm-hello-world-on-zynq-using-vitis-9fc8b7
    Xil_Out32(0xFFFFFFF0, 0x3F000000);
    dmb(); // waits until write has finished
    //    print("ARM0: sending the SEV to wake up ARM1\n\r");
    __asm__("sev");
    // GPIO Init
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

    uint32_t game_state = 0;
    static uint8_t start_col = 0;
    int32_t player_x = 384 / 2 - 16;
    int32_t player_y = 400;
#define INIT_ENEMY_HP 13385
#define INIT_PL_HP 385
#define MAX_BUL_GAME 256
    int32_t enemy_hp = INIT_ENEMY_HP;
    int32_t player_hp = INIT_PL_HP;
    uint32_t fall_down = 0;
    uint32_t fb1_alt_disp = 0;
    // frame loop/ game loop
    struct bul {
        double dir_x;
        double dir_y;
        double x;
        double y;
        int valid;
        int type;
    };
    struct bul bullets[MAX_BUL_GAME];
    struct bul pl_bullets[128];
    int32_t free_bullet_head = 0;
    int32_t free_bullet_next[MAX_BUL_GAME];
    int free_pl_bullet_head = 0;
    int free_pl_bullet_next[128];
    for (int i = 0; i < MAX_BUL_GAME; i++) {
        bullets[i].dir_x = 0;
        bullets[i].dir_y = 0;
        bullets[i].x = 0;
        bullets[i].y = 0;
        bullets[i].valid = 0;
        bullets[i].type = 0;
        free_bullet_next[i] = i + 1;
    }
    for (int i = 0; i < 128; i++) {
        pl_bullets[i].dir_x = 0;
        pl_bullets[i].dir_y = 0;
        pl_bullets[i].x = 0;
        pl_bullets[i].y = 0;
        pl_bullets[i].valid = 0;
        pl_bullets[i].type = 0;
        free_pl_bullet_next[i] = i + 1;
    }
    free_pl_bullet_next[127] = -1;
    free_bullet_next[MAX_BUL_GAME - 1] = -1;
    double theta = 0.0;
    int bullet_speed = 5;
    // center is 384/2=192, 448/2=224

    int incr_theta = 130;
    int incr_theta_dir = 2;
    double dir_xx = 0;
    double dir_yy = 0;
    int bul_type = 0;
    int is_stage2 = 0;
    int bul_change = 120;
    setMemFlag(INFORM_STAGE2, 0);
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
                setMemFlag(INFORM_READER, 1);
            }
            break;
        case 1: // playing stage
            if (!is_stage2) {
                if (getMemFlag(INFORM_STAGE2)) {
                    is_stage2 = 1;
                    setMemFlag(INFORM_STAGE2, 0);
                    setup_AUDIO(BGM, BGM_STAGE6_BOSS);
                }
            }
            get_keys();
            // add bullet
            if (!is_stage2) {

                if (incr_theta == 180) {
                    incr_theta_dir = -2;
                }
            } else {
                if (incr_theta == 480) {
                    incr_theta_dir = -2;
                }
            }
            if (incr_theta == 90) {
                incr_theta_dir = 2;
            }
            incr_theta += incr_theta_dir;
            if (frame % 1 == 0) {
                for (int k = 0; k < 1 + incr_theta / 60; k++) {
                    if (!is_stage2) {
                        dir_xx = sin(theta) * min(incr_theta, 120) / 10;
                        dir_yy = cos(theta) * min(incr_theta, 120) / 10;
                    } else {
                        dir_xx = sin(theta) * min(incr_theta, 270) / 10;
                        dir_yy = cos(theta) * min(incr_theta, 270) / 10;
                    }
                    if (free_bullet_head != -1) {
                        bullets[free_bullet_head].valid = 1;
                        bullets[free_bullet_head].x = 192;
                        bullets[free_bullet_head].y = 224;
                        bullets[free_bullet_head].dir_x = dir_xx;
                        bullets[free_bullet_head].dir_y = dir_yy;
                        if (!is_stage2) {
                            bullets[free_bullet_head].type = bul_type;
                        } else {
                            bullets[free_bullet_head].type = rand() % 16;
                        }
                        free_bullet_head = free_bullet_next[free_bullet_head];
                    }
                    theta += incr_theta * 3.14 / 720.0;
                }
            }
            if (key_z) {
                if (frame % 3 == 0) {
                    if (free_pl_bullet_head != -1) {
                        pl_bullets[free_pl_bullet_head].valid = 1;
                        pl_bullets[free_pl_bullet_head].x = player_x + 16;
                        pl_bullets[free_pl_bullet_head].y = player_y;
                        pl_bullets[free_pl_bullet_head].dir_x = 0;
                        pl_bullets[free_pl_bullet_head].dir_y = -20;
                        if (!is_stage2) {
                            pl_bullets[free_pl_bullet_head].type = 0;
                        } else {
                            pl_bullets[free_pl_bullet_head].type = 1;
                        }
                        free_pl_bullet_head = free_pl_bullet_next[free_pl_bullet_head];
                        setup_SFX_with_delay(SFX4_PL_SHOOT);
                    }
                }
            }
            // update bullet position
            for (int i = 0; i < 128; i++) {
                if (pl_bullets[i].valid) {
                    pl_bullets[i].x += pl_bullets[i].dir_x;
                    pl_bullets[i].y += pl_bullets[i].dir_y;
                    if (pl_bullets[i].x < 0 || pl_bullets[i].x > 384 || pl_bullets[i].y < 210 || pl_bullets[i].y > 448) {
                        pl_bullets[i].valid = 0;
                        free_pl_bullet_next[i] = free_pl_bullet_head;
                        free_pl_bullet_head = i;
                        enemy_hp -= 10;
                    }
                }
            }
            // if bullet is exceeding the screen then free it.
            for (int i = 0; i < MAX_BUL_GAME; i++) {
                if (bullets[i].valid) {
                    bullets[i].x += bullets[i].dir_x;
                    bullets[i].y += bullets[i].dir_y;
                    if (bullets[i].x < 0 || bullets[i].x > 384 || bullets[i].y < 0 || bullets[i].y > 448) {
                        bullets[i].valid = 0;
                        free_bullet_next[i] = free_bullet_head;
                        free_bullet_head = i;
                    }
                }
            }
            if (frame % bul_change == 0) {
                bul_type = (bul_type + 1) % 16;
                bul_change -= 4;
            }
            if (bul_change <= 0) {
                bul_change = 100;
            }
            for (int i = 0; i < MAX_BUL_GAME; i++) {
                set_enemy_bullet(i, (int)bullets[i].x, (int)bullets[i].y, bullets[i].type, bullets[i].valid);
            }
            for (int i = 0; i < 128; i++) {
                set_player_bullet(i, (int)pl_bullets[i].x, (int)pl_bullets[i].y, 0, pl_bullets[i].valid);
            }
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

            time_tick_last = get_time_tick();
            toggle_render();
            Xil_WaitForEventSet(XSCUGIC_SW_TIMEOUT_VAL, 1, &Render2DDoneCond);
            Render2DDoneCond = FALSE;
            frame++;
            time_tick = get_time_tick();
            printf("2Drt:%f ms \r\n", ((float)time_tick_last - (float)time_tick) / (float)333333);
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
                if (player_hp <= 0) {
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
            sprintf(enemy_hp_str, "Enemy HP:%d", enemy_hp);
            if (fb1_alt_disp) {
                soft_draw_board_sprite(FB1_BASE, PLAYER, 0, player_x, player_y);
                if (!is_stage2) {
                    soft_draw_board_sprite(FB1_BASE, ENEMY, 2, (384 - 32) / 2, 210);
                } else {
                    soft_draw_board_sprite(FB1_BASE, BOSS, 0, (384 - 32) / 2 - 16, 210 - 64);
                }

                fb1_alt_disp = 0;

                // draw Text
#define PLAYER_HP_X 640 - 24 * 8
#define PLAYER_HP_Y 16
#define ENEMY_HP_X 640 - 24 * 8
#define ENEMY_HP_Y 32
                clear_text(FB1_BASE, PLAYER_HP_X, PLAYER_HP_Y, 14);
                clear_text(FB1_BASE, ENEMY_HP_X, ENEMY_HP_Y, 14);
                draw_text(FB1_BASE, PLAYER_HP_X, PLAYER_HP_Y, RGB(0xFF, 0, 0), player_hp_str);
                draw_text(FB1_BASE, ENEMY_HP_X, ENEMY_HP_Y, RGB(0, 0, 0xFF), enemy_hp_str);
            } else {
                soft_draw_board_sprite(FB1_ALT_BASE, PLAYER, 0, player_x, player_y);
                if (!is_stage2) {
                    soft_draw_board_sprite(FB1_ALT_BASE, ENEMY, 2, (384 - 32) / 2, 210);
                } else {
                    soft_draw_board_sprite(FB1_ALT_BASE, BOSS, 0, (384 - 32) / 2 - 16, 210 - 64);
                }
                fb1_alt_disp = 1;
                // draw Text
                clear_text(FB1_ALT_BASE, PLAYER_HP_X, PLAYER_HP_Y, 14);
                clear_text(FB1_ALT_BASE, ENEMY_HP_X, ENEMY_HP_Y, 14);
                draw_text(FB1_ALT_BASE, PLAYER_HP_X, PLAYER_HP_Y, RGB(0xFF, 0, 0), player_hp_str);
                draw_text(FB1_ALT_BASE, ENEMY_HP_X, ENEMY_HP_Y, RGB(0, 0, 0xFF), enemy_hp_str);
            }
            // toggle fb1 alt
            toggle_fb1_alt();
            if (enemy_hp <= 0) {
                game_state = 3;
                setup_AUDIO(BGM, BGM_DEAD);
                clear_text(FB1_ALT_BASE, PLAYER_HP_X, PLAYER_HP_Y, 14);
                clear_text(FB1_BASE, ENEMY_HP_X, ENEMY_HP_Y, 14);
                clear_text(FB1_ALT_BASE, PLAYER_HP_X, PLAYER_HP_Y, 14);
                clear_text(FB1_BASE, ENEMY_HP_X, ENEMY_HP_Y, 14);
                setup_AUDIO(SFX, SFX13_ENEMY_DEAD);
            }
            break;
        case 2: // bad ending
            setMemFlag(INFORM_READER, 0);

            if (!fb1_alt_disp) {
                draw_text(FB1_BASE, 240, 240, RGB(0xFF, 0xFF, 0xFF), "You lose!");
                draw_text(FB1_BASE, 240, 260, RGB(0xFF, 0xFF, 0xFF), "Press Z to restart");
            } else {
                draw_text(FB1_ALT_BASE, 240, 240, RGB(0xFF, 0xFF, 0xFF), "You lose!");
                draw_text(FB1_ALT_BASE, 240, 260, RGB(0xFF, 0xFF, 0xFF), "Press Z to restart");
            }
            sleep(2);
            get_keys();
            if (key_z) {
                game_state = 0;
                player_hp = INIT_PL_HP;
                enemy_hp = INIT_ENEMY_HP;
                frame = 0;
                theta = 0.0;
                incr_theta = 130;
                incr_theta_dir = 2;
                dir_xx = 0;
                dir_yy = 0;
                bul_type = 0;
                player_x = 384 / 2 - 16;
                player_y = 400;
                is_stage2 = 0;
                setMemFlag(INFORM_READER, 1);
                setMemFlag(INFORM_RESTART, 1);
                sleep(1);
                setMemFlag(INFORM_STAGE2, 0);
            }
            break;

        case 3: // good ending
            setMemFlag(INFORM_READER, 0);
            if (!fb1_alt_disp) {
                draw_text(FB1_BASE, 240, 240, RGB(0xFF, 0xFF, 0xFF), "You win!");
                draw_text(FB1_BASE, 240, 260, RGB(0xFF, 0xFF, 0xFF), "Long press(3s) Z to restart");
            } else {
                draw_text(FB1_ALT_BASE, 240, 240, RGB(0xFF, 0xFF, 0xFF), "You win!");
                draw_text(FB1_ALT_BASE, 240, 260, RGB(0xFF, 0xFF, 0xFF), "Long press(3s) Z to restart");
            }
            sleep(2);
            get_keys();
            if (key_z) {
                game_state = 0;
                player_hp = INIT_PL_HP;
                enemy_hp = INIT_ENEMY_HP;
                frame = 0;
                theta = 0.0;
                incr_theta = 130;
                incr_theta_dir = 2;
                dir_xx = 0;
                dir_yy = 0;
                bul_type = 0;
                player_x = 384 / 2 - 16;
                player_y = 400;
                is_stage2 = 0;
                setMemFlag(INFORM_READER, 1);
                setMemFlag(INFORM_RESTART, 1);
                sleep(1);
                setMemFlag(INFORM_STAGE2, 0);
            }
            break;
        }
    }

    cleanup_platform();
    return 0;
}
