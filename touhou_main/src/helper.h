#ifndef HELPER
#define HELPER
#include "sleep.h"
#include "xgpiops.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xscutimer.h"
#include "xsdps.h"
#include <stdio.h>

// cf.
// https://docs.amd.com/r/en-US/ug585-zynq-7000-SoC-TRM/Register-gpio-DATA_1
#define GPIO0_OUT_BASE 0xE000A040 //
#define GPIO2_OUT_BASE 0xE000A048 // lower 32bits in EMIO
#define GPIO3_IN_BASE 0xE000A06C  // upper 32bits in EMIO

#define FB0_BASE 0x01000000
#define FB1_BASE 0x0112c000
#define FB0_ALT_BASE 0x01258000
#define FB1_ALT_BASE 0x01384000
#define AUDIO_BASE_ADDR 0x014B0000
#define AUDIO_FILE_SIZE 0x50bfe0
#define BULLET_SPRITE_BASE 0x00800000
#define GAME_INFO_BASE 0x00810000
// typedef __attribute__((packed)) struct {
//     // x 9 bit y 9bit rotation 9bit(512 degrees),type 4bit, valid 1bit
//     uint32_t x : 9;        // 0-511
//     uint32_t y : 9;        // 0-511
//     uint32_t rotation : 9; // 0-511 mapped to 0-359
//     uint32_t type : 4;     // 0-15 types
//     uint32_t valid : 1;
// } bullet_t; // in total 4B
#define FB1_START_X (33 - 1)
#define FB1_END_X (33 - 1 + 384)
#define FB1_START_Y (17 - 1)
#define FB1_END_Y (17 - 1 + 448)
#define DELAY_TIME 500 * 1000
// AUDIO MARCROs
#define BGM 1
#define SFX 0
#define BGM_START 0
#define BGM_STAGE6_PATH 1
#define BGM_STAGE6_BOSS 2
#define BGM_DEAD 3
// SDIO MARCROs
#define NUM_BLOCKS (491520 + 524288) // Two volumes
// #define NUM_BLOCKS (2400)//Two volumes
#define SECTOR_OFFSET 0
// Fb draw

#define LINE_STRIDE_BYTE 2560
// input 16bit, output 32bit
#define RGB_DITHER(x)                                                                                                            \
    ((((x >> (1 + 2 * 5)) & 0x1F) << (24 + 3)) | (((x >> (1 + 1 * 5)) & 0x1F) << (16 + 3)) | (((x >> 1) & 0x1F) << (8 + 3)) | 0x0)

#define RGB(r, g, b) ((r << 24) | (g << 16) | (b << 8))
// Binding
extern uint8_t font_rom[128*16];
#define GPIO0_OUT *((volatile uint32_t *)GPIO0_OUT_BASE)
#define GPIO2_OUT *((volatile uint32_t *)GPIO2_OUT_BASE)
#define GPIO3_IN *((volatile uint32_t *)GPIO3_IN_BASE)
#define KEYS_TOUHOU ((GPIO3_IN >> 2) & 0x3F) // bit [7:2], 6 keys
void setup_AUDIO(uint32_t is_BGM, uint32_t audio_type);
void set_audio_volume(uint8_t volume_shift);
int SDIO_Read(uint32_t dest_mem_addr, uint32_t start_sector, uint32_t sector_count);
void toggle_fb0_alt();
void toggle_fb1_alt();
void toggle_render();
void clear_fb(void *fb_ptr);
#define clear_fb0() clear_fb(FB0_BASE)
#define clear_fb0_alt() clear_fb(FB0_ALT_BASE)
#define clear_fb1() clear_fb(FB1_BASE)
#define clear_fb1_alt() clear_fb(FB1_ALT_BASE)
void set_die_buzzer();
void clear_die_buzzer();
uint32_t compose_entity(uint32_t X, uint32_t Y, uint32_t ROT, uint32_t TYPE, uint32_t VALID);
void copy_bullet_sprite_to_dest();
void test_write_game_info();
void setup_timer();
uint32_t get_time_tick();
void clear_enemy_bullet();
void map_enemy_bullet_to_vram();
void set_enemy_bullet(uint32_t idx, uint32_t x, uint32_t y, uint32_t type);
void invalidate_enemy_bullet(uint32_t idx);
void draw_text(void *fb_ptr, int x, int y, uint32_t color, char *text);
#endif
