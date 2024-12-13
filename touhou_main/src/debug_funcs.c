
#include "debug_funcs.h"
#include "helper.h"
#include "intr.h"
#include "structure.h"
void test_keys() {
    while (1) {
        xil_printf("TouhouPins:%x\r\n", KEYS_TOUHOU);
    }
}
// used in board
void draw_board_strips(void *fb_ptr) {
    volatile uint32_t *ptr;
    uint8_t red, green, blue;
    for (int i = FB1_START_Y; i < FB1_END_Y; i++) {
        ptr = i * LINE_STRIDE_BYTE + fb_ptr + FB1_START_X * 4;
        red = 25;
        green = 25;
        blue = 25;
        for (int j = FB1_START_X; j < FB1_END_X; j++) {
            red = (red + 1) % 256;
            green = (green + j / 3) % 256;
            blue = (blue + j / 5) % 256;
            *ptr = (red << 24) | (green << 16) | (blue << 8) | (1); // bit 1 to enable drawing in fb1
            ptr += 0x1;
        }
    }
    Xil_DCacheFlushRange(fb_ptr, LINE_STRIDE_BYTE * 640);
    xil_printf("Strip Done\r\n");
}
// used in board
void draw_board_color(void *fb_ptr, uint32_t color) {
    volatile uint32_t *ptr;
    uint8_t red, green, blue;
    for (int i = FB1_START_Y; i < FB1_END_Y; i++) {
        ptr = i * LINE_STRIDE_BYTE + fb_ptr + FB1_START_X * 4;
        for (int j = FB1_START_X; j < FB1_END_X; j++) {
            *ptr = color | 0x1; // bit 1 to enable drawing in fb1
            ptr++;
        }
    }
    Xil_DCacheFlushRange(fb_ptr, LINE_STRIDE_BYTE * 640);
    xil_printf("Draw Color %x Done\r\n", color);
}
// only for debug
void debug_console() {
    uint32_t volume = 5, command, bgm = 0;
    while (1) {
        xil_printf("GPIO3:input:%x\r\n", GPIO3_IN); // scan input
        xil_printf("Please enter new command:\r\n");
        scanf("%d", &command);
        // bit 0 done, bit 1 idle, bit 2 ready
        if (command == 1) {
            toggle_fb0_alt();
        } else if (command == 2) {
            set_die_buzzer();
        } else if (command == 3) {
            clear_die_buzzer();
        } else if (command == 4) {
            xil_printf("input new volume:\r\n");
            scanf("%d", &volume);
            set_audio_volume(volume);
        } else if (command == 5) {
            xil_printf("input new BGM:\r\n");
            scanf("%d", &bgm);
            setup_AUDIO(BGM, bgm);
        } else if (command == 6) {
            xil_printf("Issue new draw!\r\n");
            toggle_render();
        }
    }
}
void soft_draw_board_sprite(void *fb_ptr, type_object category, int type, int dest_x, int dest_y) {
    sprite_t dest_sprite_info;
    switch (category) {
    case ENEMY_BULLET:
        dest_sprite_info = get_enemy_bullet_info(type);
        break;
    case PLAYER_BULLET:
        dest_sprite_info = get_player_bullet_info(type);
        break;
    case ENEMY:
        dest_sprite_info = get_enemy_info(type);
        break;
    case PLAYER:
        dest_sprite_info = get_player_info();
        break;
    case BOSS:
        dest_sprite_info = get_boss_info();
        break;
    case ITEM:
        dest_sprite_info = get_item_info(type);
        break;
    default:
        xil_printf("Invalid category %d\r\n", category);
        return;
    }
    uint32_t width = dest_sprite_info.width;
    uint32_t height = dest_sprite_info.height;
    uint32_t x = dest_sprite_info.x;
    uint32_t y = dest_sprite_info.y;
    volatile uint32_t *ptr;
    volatile uint16_t *sprite_ptr = (uint16_t *)BULLET_SPRITE_BASE;

    for (int i = 0; i < height; i++) {
        ptr = fb_ptr + (FB1_START_Y + dest_y + i) * LINE_STRIDE_BYTE + (FB1_START_X + dest_x) * 4;
        for (int j = 0; j < width; j++) {
            uint16_t sampled_col = sprite_ptr[(i + y) * 256 + (j + x)];
            if ((sampled_col & 0xFFFE) != 0x1204) {
                // skip transparent color(magic number RGB 11 45 14)
                // hex((0x14 >> 3) << 11 | (0x45 >> 3) << 6 | (0x14 >> 3) << 1) = 0x1204
                *ptr = RGB_DITHER(sampled_col) | 0x1; // bit 1 to enable drawing in fb1
            }
            ptr++;
        }
    }
    Xil_DCacheFlushRange(fb_ptr, LINE_STRIDE_BYTE * 640);
}
void test_bullet_map(void *fb_ptr) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 2; j++) {
            soft_draw_board_sprite(fb_ptr, ENEMY_BULLET, (i + j * 8) % 16, 32 * i, 32 * j);
        }
    }
    for (int i = 0; i < 4; i++) {
        soft_draw_board_sprite(fb_ptr, PLAYER_BULLET, i, 64 * i, 64);
    }
    for (int i = 0; i < 8; i++) {
        soft_draw_board_sprite(fb_ptr, ITEM, i % 6, 32 * i, 128);
    }
    for (int i = 0; i < 4; i++) {
        soft_draw_board_sprite(fb_ptr, ENEMY, i % 3, 64 * i, 128 + 32);
    }
    soft_draw_board_sprite(fb_ptr, PLAYER, 0, 0, 128 + 32 + 64);
    soft_draw_board_sprite(fb_ptr, BOSS, 0, 64, 128 + 32 + 64);
}
void test_draw2d_time() {
    uint32_t time_tick = 0, time_tick_last = 0;
    time_tick = get_time_tick();
    toggle_render();
    Xil_WaitForEventSet(XSCUGIC_SW_TIMEOUT_VAL, 1, &Render2DDoneCond);
    time_tick_last = time_tick;
    time_tick = get_time_tick();
    Render2DDoneCond = FALSE; // need to clear it
    // note that timer decreases.
    xil_printf("2D Render done! used time:%d\r\n", (time_tick_last - time_tick) / 333333);
}
void test_60frame_time() {
    uint32_t time_tick = 0, time_tick_last = 0;
    uint32_t frame = 0;
    while (1) {
        Xil_WaitForEventSet(XSCUGIC_SW_TIMEOUT_VAL, 1, &InitNewFrameCond);
        InitNewFrameCond = FALSE; // need to clear it
        if (frame == 0) {
            time_tick = get_time_tick();
        }
        frame++;
        if (frame % (61) == 0) {
            time_tick_last = time_tick;
            time_tick = get_time_tick();
            xil_printf("Draw 60 frames, time_diff:%d\r\n", (time_tick_last - time_tick) / 333333);
            return;
        }
    }
}