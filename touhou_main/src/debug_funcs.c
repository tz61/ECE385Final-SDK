
#include "debug_funcs.h"
#include "helper.h"
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