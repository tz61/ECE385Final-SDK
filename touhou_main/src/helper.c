#include "helper.h"
#include "structure.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include <stdio.h>
extern uint16_t Bullet_sprite[128 * 256];
uint8_t key_z, key_shift, key_up, key_down, key_left, key_right;
XScuTimer_Config *ConfigPtr;
XScuTimer Timer;
XScuTimer *TimerInstance = &Timer;
void setup_timer() {
    ConfigPtr = XScuTimer_LookupConfig(XPAR_XSCUTIMER_0_DEVICE_ID);
    XScuTimer_CfgInitialize(TimerInstance, ConfigPtr, ConfigPtr->BaseAddr);
    XScuTimer_LoadTimer(TimerInstance, 0xFFFFFFFF);
    XScuTimer_EnableAutoReload(TimerInstance);
    XScuTimer_Start(TimerInstance);
}
uint32_t get_time_tick() { return XScuTimer_GetCounterValue(TimerInstance); }
void copy_bullet_sprite_to_dest() {
    // 0x 0080 0000
    xil_printf("Copying bullet sprite to dest\r\n");
    volatile uint16_t *dest = (uint16_t *)BULLET_SPRITE_BASE;
    for (int i = 0; i < 128 * 256; i++) {
        *dest = Bullet_sprite[i];
        dest++;
    }
    Xil_DCacheFlushRange(BULLET_SPRITE_BASE, 128 * 256 * 2);
    xil_printf("Copy done\r\n");
    xil_printf("color at (8,8):%04x\r\n", Bullet_sprite[256 * 8 + 8]);
    xil_printf("color at (8,8) in mem:%04x\r\n", *((volatile uint16_t *)(BULLET_SPRITE_BASE + 2 * (256 * 8 + 8))));
}
typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t type;
    uint32_t valid;
} bullet_t;
uint32_t compose_entity(uint32_t X, uint32_t Y, uint32_t ROT, uint32_t TYPE, uint32_t VALID) {
    return ((X) | ((Y) << 9) | ((ROT) << 18) | ((TYPE) << 27) | ((VALID) << 31));
}
void clear_bullet() {
    volatile uint32_t *game_info = (uint32_t *)GAME_INFO_BASE;
    // clear all enemy bullet
    for (int i = 0; i < 2048; i++) {
        game_info[i] = compose_entity(0, 0, 0, 0, 0);
    }
    Xil_DCacheFlushRange(GAME_INFO_BASE, (2048) * 4);
}
void set_enemy_bullet(uint32_t idx, uint32_t x, uint32_t y, uint32_t type) {
    volatile uint32_t *game_info = (uint32_t *)GAME_INFO_BASE;
    game_info[idx] = compose_entity(x, y, 0, type, 1);
    Xil_DCacheFlushRange(GAME_INFO_BASE, (2048) * 4);
}
void set_player_bullet(uint32_t idx, uint32_t x, uint32_t y, uint32_t type) {
    volatile uint32_t *game_info = (uint32_t *)GAME_INFO_BASE;
    game_info[1536 + idx] = compose_entity(x, y, 0, type, 1);
    Xil_DCacheFlushRange(GAME_INFO_BASE, (2048) * 4);
}
void test_write_game_info() {
    // 0x 0081 0000
    xil_printf("Writing test game info\r\n");
    volatile uint32_t *dest = (uint32_t *)GAME_INFO_BASE;
    for (int i = 0; i < 2048; i++) {
        *dest = 0;
        dest++;
    }
    dest = (uint32_t *)GAME_INFO_BASE;
    // enemy bullet
    // dest[0] = compose_entity(8, 8, 0, 0, 1);
    // player bullet
    // dest[1536] = compose_entity(0, 0, 0, 3, 1);
    Xil_DCacheFlushRange(GAME_INFO_BASE, (2048) * 4);
    xil_printf("Write test done\r\n");
}

void setup_AUDIO(uint32_t is_BGM, uint32_t audio_type) {
    GPIO2_OUT |= (((is_BGM << 4) + audio_type) << 4); // [8:4], bit [8] BGM =1
    GPIO2_OUT |= 0x8;                                 // toggle bit 3
    GPIO2_OUT &= ~0x8;
    GPIO2_OUT &= ~(0x1F << 4); // clear BGM bit and type bit for next set
}
void set_audio_volume(uint8_t volume_shift) {
    GPIO2_OUT &= ~(0x7); // [2:0]
    GPIO2_OUT |= (volume_shift & 0x7);
}
void toggle_fb0_alt() {
    // default 0(not alt)
    static uint8_t fb0_alt = 1;
    // bit 9
    if (fb0_alt) {
        GPIO2_OUT |= 0x1 << 9;
        fb0_alt = 0;
    } else {
        GPIO2_OUT &= ~(0x1 << 9);
        fb0_alt = 1;
    }
}
void toggle_fb1_alt() {
    // default 0(not alt)
    static uint8_t fb1_alt = 1;
    // bit 10
    if (fb1_alt) {
        GPIO2_OUT |= 0x1 << 10;
        fb1_alt = 0;
    } else {
        GPIO2_OUT &= ~(0x1 << 10);
        fb1_alt = 1;
    }
}

void toggle_render() {
    // bit 11
    GPIO2_OUT |= 0x1 << 11;
    GPIO2_OUT &= ~(0x1 << 11);
}
void clear_fb(void *fb_ptr) {
    volatile uint32_t *ptr;
    ptr = fb_ptr;
    for (int i = 0; i < 480; i++) {
        for (int j = 0; j < 640; j++) {
            *ptr = 0x00000000;
            //  xil_printf("%x",board[i*640+j]);
            ptr++;
        }
    }
    Xil_DCacheFlushRange(fb_ptr, LINE_STRIDE_BYTE * 640);
}
void clear_board(void *fb_ptr) {
    volatile uint32_t *ptr;
    ptr = fb_ptr;
    for (int i = FB1_START_Y; i < FB1_END_Y; i++) {
        ptr = i * LINE_STRIDE_BYTE + fb_ptr + FB1_START_X * 4;
        for (int j = FB1_START_X; j < FB1_END_X; j++) {
            *ptr = 0x00FF0000;
            ptr++;
        }
    }
    Xil_DCacheFlushRange(fb_ptr, LINE_STRIDE_BYTE * 640);
}
void set_die_buzzer() {
    // bit 12
    GPIO2_OUT |= 0x1 << 12;
}
void clear_die_buzzer() {
    // bit 12
    GPIO2_OUT &= ~(0x1 << 12);
}
int SDIO_Read(uint32_t dest_mem_addr, uint32_t start_sector, uint32_t sector_count) {
    static XSdPs SdInstance;
    XSdPs_Config *SdConfig;
    uint32_t sector_read = 0;
    int Status;
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

    Status = XSdPs_CfgInitialize(&SdInstance, SdConfig, SdConfig->BaseAddress);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    Status = XSdPs_CardInitialize(&SdInstance);
    if (Status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    if (!(SdInstance.HCS)) {
        start_sector *= XSDPS_BLK_SIZE_512_MASK;
        sector_read *= XSDPS_BLK_SIZE_512_MASK;
    }

    /*
     * Read data from SD/eMMC.
     */
    char loading_str[40];
    uint32_t start_load_bgm = 0;
#define MAX_SINGLE_READ_SECTOR_COUNT (2 * 1024 * 1024 / 512)
    xil_printf("\r\n SDIO Started reading!\r\n");
    while (sector_read < sector_count) {
        sprintf(loading_str, "[Loading]Reading sectors:%d/%d", sector_read, sector_count);
        draw_text(FB1_BASE, 160, 400, RGB(0x66, 0xCC, 0xFF), loading_str);
        xil_printf("\rReading %d/%d", sector_read, sector_count);
        Status = XSdPs_ReadPolled(&SdInstance, start_sector + sector_read, MAX_SINGLE_READ_SECTOR_COUNT,
                                  dest_mem_addr + sector_read * 512);
        if (Status == XST_SUCCESS) {
            // Max 2MiB in a single poll, 2*1024*1024/512=4096 sectors max
            // only 4096 sector. within a poll
            sector_read += MAX_SINGLE_READ_SECTOR_COUNT;
        }
        if (sector_read >= 20000) {
            if (!start_load_bgm) {
                start_load_bgm = 1;
                // Setup BGM
                set_audio_volume(4);
                setup_AUDIO(BGM, BGM_START);
            }
        }
    }
    clear_text(FB1_BASE, 160, 400, 40);
    xil_printf("\r\n SDIO Read done!\r\n");

    return XST_SUCCESS;
}

void draw_text(void *fb_ptr, int x, int y, uint32_t color, char *text) {
    volatile uint32_t *ptr;
    for (int i = 0; text[i] != '\0'; i++) {
        ptr = fb_ptr + y * LINE_STRIDE_BYTE + (x + i * 8) * 4;
        char c = text[i];
        for (int j = 0; j < 16; j++) { // 16 vertical lines, each line 8 pixels
            uint8_t cur_font = font_rom[c * 16 + j];
            // LSB is the leftmost pixel
            for (int k = 0; k < 8; k++) {
                if ((cur_font & 0x1)) {
                    *ptr = color | 0x1; // bit 1 to enable drawing in fb1
                } else {
                    *ptr = 0;
                }
                cur_font >>= 1;
                ptr++;
            }
            ptr = ptr - 8 + LINE_STRIDE_BYTE / 4;
        }
    }
    Xil_DCacheFlushRange(fb_ptr, LINE_STRIDE_BYTE * 640);
}
void clear_text(void *fb_ptr, int x, int y, int length) {
    volatile uint32_t *ptr;
    for (int i = 0; i < length; i++) {
        ptr = fb_ptr + y * LINE_STRIDE_BYTE + (x + i * 8) * 4;
        for (int j = 0; j < 16; j++) { // 16 vertical lines, each line 8 pixels
            for (int k = 0; k < 8; k++) {
                *ptr = 0; // clear everything
                ptr++;
            }
            ptr = ptr - 8 + LINE_STRIDE_BYTE / 4;
        }
    }
    Xil_DCacheFlushRange(fb_ptr, LINE_STRIDE_BYTE * 640);
}

void get_keys() {
    uint32_t tmp = KEYS_TOUHOU;
    // clear all keys
    key_z = 0;
    key_shift = 0;
    key_up = 0;
    key_down = 0;
    key_left = 0;
    key_right = 0;
    if (tmp & 0x1) {
        key_z = 1;
    }
    if (tmp & 0x2) {
        key_shift = 1;
    }
    if (tmp & 0x4) {
        key_up = 1;
    }
    if (tmp & 0x8) {
        key_down = 1;
    }
    if (tmp & 0x10) {
        key_left = 1;
    }
    if (tmp & 0x20) {
        key_right = 1;
    }
}
void go_menu() {
    static int8_t volume = 4;
    get_keys();
    if (key_shift && key_right) {
        xil_printf("Go to menu!\r\n");
        setup_AUDIO(SFX, SFX1_MENU_PAUSE);
        usleep(200 * 1000);
        while (1) {
            get_keys();
            if (key_shift && key_left) {

                clear_text(FB1_BASE, MENU_X_OFFSET, 0, 20);
                clear_text(FB1_BASE, MENU_X_OFFSET, 32, 20);
                clear_text(FB1_BASE, MENU_X_OFFSET, 64, 20);
                clear_text(FB1_BASE, MENU_X_OFFSET, 96, 20);
                setup_AUDIO(SFX, SFX0_MENU_CANCEL);
                xil_printf("Exit menu!\r\n");
                break;
            }
            if (key_left) {
                xil_printf("Decrease volume!\r\n");
                volume += 1;
                if (volume == 8) {
                    volume = 7;
                }
                set_audio_volume(volume);
            }
            if (key_right) {
                xil_printf("Increase volume!\r\n");
                volume -= 1;
                if (volume == -1) {
                    volume = 0;
                }
                set_audio_volume(volume);
            }
            // Draw menu entrys
            draw_text(FB1_BASE, MENU_X_OFFSET, 0, MENU_INACTIVE_COLOR, "Menu:");
            draw_text(FB1_BASE, MENU_X_OFFSET, 32, MENU_INACTIVE_COLOR, "Left:-V");
            draw_text(FB1_BASE, MENU_X_OFFSET, 64, MENU_INACTIVE_COLOR, "Right:+V");
            char volume_str[21];
            sprintf(volume_str, "Volume[0-7]:%d", 7 - volume);
            draw_text(FB1_BASE, MENU_X_OFFSET, 96, MENU_ACTIVE_COLOR, volume_str);
            // sleep when key pressed
            if (key_z | key_shift | key_up | key_down | key_left | key_right) {
                // setup_AUDIO(SFX, SFX2_MENU_OK); // too noisy
                usleep(200 * 1000);
            }
        }
    }
}

// read one frame animation to FB0_BASE
void ReadAnimation() {
    static XSdPs SdInstance;
    XSdPs_Config *SdConfig;
    static uint32_t sector_read = 0;
    int Status;
    static int is_init = 0;
    if (!is_init) {
        SdConfig = XSdPs_LookupConfig(XPAR_XSDPS_0_DEVICE_ID);
        XSdPs_CfgInitialize(&SdInstance, SdConfig, SdConfig->BaseAddress);
        XSdPs_CardInitialize(&SdInstance);
        is_init = 1;
    }
    uint32_t time_tick_last,time_tick;
    if (sector_read < 10863 * 2400) { // 10863 frames
    	time_tick_last = get_time_tick();
        Status = XSdPs_ReadPolled(&SdInstance, 1015808 + sector_read, 2400, (u8 *)FB0_BASE);
        time_tick = get_time_tick();
        printf("SD Read:%f ms \r\n",((float)time_tick_last-(float)time_tick)/(float)333333);
        if (Status == XST_SUCCESS) {
            sector_read += 4800; // skip 1 frame
        }
    } else {
        // reset(loop)
        sector_read = 0;
    }
}
uint8_t getMemFlag(volatile uint32_t* memAddr){
	Xil_DCacheInvalidateRange(memAddr,1);
	return *memAddr;
}
void setMemFlag(volatile uint32_t* memAddr,uint8_t val){
	Xil_DCacheInvalidateRange(memAddr,1);
	*memAddr=val;
}
