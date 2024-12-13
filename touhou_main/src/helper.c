#include "helper.h"
#include "structure.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include <stdio.h>
extern uint16_t Bullet_sprite[128 * 256];
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
bullet_t enemy_bullet[2048];
bullet_t player_bullet[256];
void clear_enemy_bullet() {
    // clear all enemy bullet
    for (int i = 0; i < 2048; i++) {
        enemy_bullet[i].type = 0;
        enemy_bullet[i].x = 0;
        enemy_bullet[i].y = 0;
        enemy_bullet[i].valid = 0;
    }
}
void set_enemy_bullet(uint32_t idx, uint32_t x, uint32_t y, uint32_t type) {
    enemy_bullet[idx].x = x;
    enemy_bullet[idx].y = y;
    enemy_bullet[idx].type = type;
    enemy_bullet[idx].valid = 1;
}
void invalidate_enemy_bullet(uint32_t idx) { enemy_bullet[idx].valid = 0; }
void map_enemy_bullet_to_vram() {
    static uint16_t bucket[12 * 14]; // bucket for each tile, max 16 bullets for each tile
    // clear bucket
    for (int i = 0; i < 12 * 14; i++) {
        bucket[i] = 0;
    }
    volatile uint32_t *dest_vram = (uint32_t *)GAME_INFO_BASE;
    for (int i = 0; i < 2688; i++) {
        *dest_vram = 0;
        dest_vram++;
    }
    // reset position
    dest_vram = (uint32_t *)GAME_INFO_BASE;
    for (int i = 0; i < 2048; i++) {
        bullet_t tmp_b = enemy_bullet[i];
        if (tmp_b.valid) {
            sprite_t info = get_enemy_bullet_info(tmp_b.type);
            // get this bullet's overlapping tile
            // rectangle (x,y,w,h) = (tmp_b.x,tmp_b.y,info.width,info.height)
            // each tile 32x32
            // 12(x)x14(y) tiles

            // left/top corner
            int tile_x = tmp_b.x / 32;
            int tile_y = tmp_b.y / 32;
            // right/bottom corner
            int tile_x_end = (tmp_b.x + info.width) / 32;
            int tile_y_end = (tmp_b.y + info.height) / 32;
            for (int k = tile_x; k <= tile_x_end; k++) {
                for (int l = tile_y; l <= tile_y_end; l++) {
                    int tile_idx = l * 12 + k;
                    // find a empty slot in this tile
                    if (bucket[tile_idx] < 16) {
                        dest_vram[tile_idx * 16 + bucket[tile_idx]] = compose_entity(tmp_b.x, tmp_b.y, 0, tmp_b.type, 1);
                        bucket[tile_idx]++;
                    } // else just ignore this bullet
                }
            }
        }
    }
    Xil_DCacheFlushRange(GAME_INFO_BASE, (12 * 14 * (16 + 8) + 64) * 4);
}
void map_player_bullet_to_vram() {
    volatile uint32_t *dest_vram = (uint32_t *)GAME_INFO_BASE;
    for (int i = 2688; i < 4032; i++) {
        *dest_vram = 0;
        dest_vram++;
    }
    for (int i = 0; i < 2048; i++) {
        bullet_t tmp_b = enemy_bullet[i];
        if (tmp_b.valid) {
            sprite_t info = get_enemy_bullet_info(tmp_b.type);
        }
    }
}
uint32_t compose_entity(uint32_t X, uint32_t Y, uint32_t ROT, uint32_t TYPE, uint32_t VALID) {
    return ((X) | ((Y) << 9) | ((ROT) << 18) | ((TYPE) << 27) | ((VALID) << 31));
}
void test_write_game_info() {
    // 0x 0081 0000
    xil_printf("Writing test game info\r\n");
    volatile uint32_t *dest = (uint32_t *)GAME_INFO_BASE;
    for (int i = 0; i < 4096; i++) {
        *dest = 0;
        dest++;
    }
    dest = (uint32_t *)GAME_INFO_BASE;
    Xil_DCacheFlushRange(GAME_INFO_BASE, (12 * 14 * (16 + 8) + 64) * 4);
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
#define MAX_SINGLE_READ_SECTOR_COUNT (2 * 1024 * 1024 / 512)
    xil_printf("\r\n SDIO Started reading!\r\n");
    while (sector_read < sector_count) {
        xil_printf("\rReading %d/%d", sector_read, sector_count);
        Status = XSdPs_ReadPolled(&SdInstance, start_sector + sector_read, MAX_SINGLE_READ_SECTOR_COUNT,
                                  dest_mem_addr + sector_read * 512);
        if (Status == XST_SUCCESS) {
            // Max 2MiB in a single poll, 2*1024*1024/512=4096 sectors max
            // only 4096 sector. within a poll
            sector_read += MAX_SINGLE_READ_SECTOR_COUNT;
        }
    }
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
                }
                cur_font >>= 1;
                ptr++;
            }
            ptr = ptr - 8 + LINE_STRIDE_BYTE / 4;
        }
    }
    Xil_DCacheFlushRange(fb_ptr, LINE_STRIDE_BYTE * 640);
}